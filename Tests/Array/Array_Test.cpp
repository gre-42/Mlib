#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

void test_array_index() {
    Array<float> a = random_array<float>(ArrayShape{5, 3}, 1);
    assert_allclose(a, Array<float>{{1, 74, 45},
                                    {262, 121, 314},
                                    {525, 222, 41},
                                    {154, 205, 582},
                                    {761, 594, 85}});
    assert_allclose(a[2], Array<float>{525, 222, 41});
    assert_isclose(a[2](1), 222.f);
    assert_isclose(a(2, 1), 222.f);
    assert_isclose(a(ArrayShape{2, 1}), 222.f);
    assert_allclose(a[ArrayShape{2}], Array<float>{525, 222, 41});
    assert_isclose(a[ArrayShape{2, 1}](), 222.f);
}

struct Data {
    float v;
};

void test_data() {
    Data data;
    data.v = 5;
    Array<Data> t;

    t.resize(3);
    assert_isclose<float>(t.ndim(), 1);
    t(2) = data;
    assert_isclose<float>(t(2).v, 5);

    t.resize[5][5](3);
    assert_isclose<float>(t.ndim(), 3);
    t[3](3, 2) = data;
    assert_isclose<float>(t.ndim(), 3);
    assert_isclose<float>(t[3](3, 2).v, 5);

    assert_isclose<float>(t[1].flattened().nelements(), 15);
}

void test_savetxt2d_loadtxt() {
    Array<float> a = random_array<float>(ArrayShape{3, 4});
    a.save_txt_2d("savetxt2d_test.m");
    Array<float> b = Array<float>::load_txt_2d("savetxt2d_test.m");
    assert_allclose(a, b);
    if (std::remove("savetxt2d_test.m") != 0) {
        throw std::runtime_error(std::strerror(errno));
    }
}

void test_row_range() {
    Array<float> a = random_array<float>(ArrayShape{3, 4});
    a.row_range(1, 3)[0](1) = -432;
    assert_isclose(a[1](1), -432.f);
}

void test_vH() {
    Array<std::complex<float>> a = (
        uniform_random_array<std::complex<float>>(ArrayShape{5, 3}, 1) +
        std::complex<float>(0, 1) *
        uniform_random_array<std::complex<float>>(ArrayShape{5, 3}, 2));
    Array<std::complex<float>> b = (
        uniform_random_array<std::complex<float>>(ArrayShape{5, 3}, 3) +
        std::complex<float>(0, 1) *
        uniform_random_array<std::complex<float>>(ArrayShape{5, 3}, 4));

    assert_allclose(
        lstsq_chol(a, b),
        lstsq(a, b),
        1e-3);
}

void test_sparse_array() {
    Array<float> r1;
    Array<float> r2;
    Array<float> r3;
    {
        Array<float> a{ArrayShape{6, 5}};
        Array<float> b{ArrayShape{6, 4}};
        a = 0;
        b = 0;
        a(1, 3) = 1;
        b(1, 3) = 1;
        r1 = dot(a.H(), b);
    }
    {
        SparseArrayCcs<float> a{ArrayShape{6, 5}};
        SparseArrayCcs<float> b{ArrayShape{6, 4}};

        a(1, 3) = 1;
        b(1, 3) = 1;
        r2 = dot2d(a.vH(), b);
    }
    assert_allclose(r1, r2);
    {
        SparseArrayCcs<float> a{ArrayShape{6, 5}};
        Array<float> b{ArrayShape{6, 4}};
        b = 0;
        a(1, 3) = 1;
        b(1, 3) = 1;
        r3 = dot2d(a.vH(), b);
    }
    assert_allclose(r1, r3);
    {
        SparseArrayCcs<float> a{ArrayShape{6, 5}};
        Array<float> ad = uniform_random_array<float>(a.shape(), 2);
        Array<float> b = uniform_random_array<float>(ArrayShape{6, 4}, 1);
        for(size_t r = 0; r < a.shape(0); ++r) {
            for(size_t c = 0; c < a.shape(1); ++c) {
                a(r, c) = ad(r, c);
            }
        }
        assert_allclose(lstsq_chol(a, b), lstsq_chol(ad, b));
        a.casted<double>();
        a.is_defined();
    }
}

void test_move() {
    Array<float> a;
    a = Array<float>{1, 2, 3};
    a.resize(2);
    assert_isclose<float>(a.length(), 2);
}

void test_element_iterable() {
    Array<float> a{2, 3, 4};
    std::list<float> l;
    for(const float& v : a.element_iterable()) {
        l.push_back(v);
    }
    assert_allclose(Array<float>{l}, a);
}

void test_save_binary() {
    {
        Array<float> a{2, 3, 4};
        a.save_binary("binary.array");
        assert_allclose(a, Array<float>::load_binary("binary.array"));
        fs::remove("binary.array");
    }
    {
        Array<float> a = uniform_random_array<float>(ArrayShape{4, 5, 6}, 1);
        a.save_binary("binary.array");
        assert_allclose(a, Array<float>::load_binary("binary.array"));
        fs::remove("binary.array");
    }
}

void test_take() {
    assert_allclose(
        Array<size_t>{{2, 3, 4}, {1, 2, 3}}.take(Array<float>{10, 11, 12, 13, 14, 15, 16}),
        Array<float>{{12, 13, 14}, {11, 12, 13}});
}

void test_fixed_array() {
    FixedArray<bool, 1, 2> f;
    f(0, 0) = true;
    f(0, 1) = true;
    assert_isequal(all(f), true);
    assert_isequal(any(f), true);
    f(0, 0) = true;
    f(0, 1) = false;
    assert_isequal(all(f), false);
    assert_isequal(any(f), true);
    f(0, 0) = false;
    f(0, 1) = true;
    assert_isequal(all(f), false);
    assert_isequal(any(f), true);
    f(0, 0) = false;
    f(0, 1) = false;
    assert_isequal(all(f), false);
    assert_isequal(any(f), false);

    FixedArray<float, 3> g{2, 3, 1.1};
    assert_isequal((g * 5.f)(1), 15.f);
}

void test_fixed_array_initialization() {
    FixedArray<int, 2> f{4, 5};
    assert_isequal(f(0), 4);
    assert_isequal(f(1), 5);
}

void test_fixed_array_slicing() {
    FixedArray<int, 2> f{4, 5};
    assert_isequal(f.row_range<1, 2>()(0), 5);
    FixedArray<int, 3, 3> kif;
    kif.row_range<0, 2>();
    kif.reshaped<2, 2>();
}

void test_append() {
    Array<float> a{ArrayShape{0}};
    a.append(5);
    a.append(6);
    a.append(7);
    a.append(8);
    a.append(9);
    a.append(10);
    assert_allclose(a, Array<float>{5, 6, 7, 8, 9, 10});
}

void test_copy() {
    Array<float> a{1, 2, 3};
    Array<float> b = a;
    Array<float> c = a.copy();
    b(0) = 5;
    assert_allclose(a, Array<float>{5, 2, 3});
    c(0) = 7;
    assert_allclose(a, Array<float>{5, 2, 3});
}

void test_sparse_array2() {
    SparseArrayCcs<float> a{ArrayShape{6, 5}};

    a(1, 3) = 1;

    assert_allclose(
        a.row_is_defined().casted<float>(),
        Array<float>{0, 1, 0, 0, 0, 0});
}

int main(int argc, char **argv) {
    test_array_index();
    test_data();
    test_savetxt2d_loadtxt();
    test_row_range();
    test_vH();
    test_sparse_array();
    test_sparse_array2();
    test_move();
    test_element_iterable();
    test_save_binary();
    test_take();
    test_fixed_array();
    test_fixed_array_initialization();
    test_fixed_array_slicing();
    test_append();
    test_copy();
    return 0;
}
