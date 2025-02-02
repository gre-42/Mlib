#include <Mlib/Images/Registration.hpp>

using namespace Mlib;

void test_patch_registration_1d_0() {
    Array<float> a = random_array<float>(ArrayShape{15});
    Array<float> b = random_array<float>(ArrayShape{15});
    ArrayShape max_window_shape{3};

    assert_isclose(a(0), b(0));
    Array<float> flow = patch_registration(a, b, max_window_shape, false);
    //lerr() << "a: " << a;
    //lerr() << "flow: " << flow;
    assert_isclose<float>(flow(0, 2), 0);
}

void test_patch_registration_1d_1() {
    Array<float> a = random_array<float>(ArrayShape{15}, 316);
    Array<float> b = random_array<float>(ArrayShape{15});
    ArrayShape max_window_shape{3};

    assert_isclose(a(0), b(2));
    {
        Array<float> flow = patch_registration(a, b, max_window_shape, false);
        assert_isclose<float>(flow(0, 2), 2);
    }
    {
        Array<float> flow = patch_registration(b, a, max_window_shape, false);
        assert_isclose<float>(flow(0, 2), -2);
        assert_allclose(flow[0], Array<float>{-2, -2, -2, -2, -2, -2, -2, -2, -2});
    }
    {
        Array<float> flow = patch_registration(b, a, max_window_shape, true);
        assert_allclose(flow[0], Array<float>{NAN, NAN, NAN, -2, -2, -2, -2, -2, -2, -2, -2, -2, NAN, NAN, NAN});
    }
}

void test_patch_registration_2d() {
    ArrayShape array_shape{15, 20};
    Array<float> a = random_array<float>(array_shape, 1);
    Array<float> b(array_shape);
    ArrayShape max_window_shape{3, 4};

    for (size_t s = 0; s <= 2; ++s) {
        b = 0;
        for (size_t i = s; i < b.shape(0); i++) {
            b[i] = a[i - s];
        }
        Array<float> flow = patch_registration(a, b, max_window_shape, false);
        //lerr() << "flow";
        //lerr() << flow;
        assert_isclose<float>(flow[0](1, 2), (float)s);
        assert_isclose<float>(flow[1](1, 2), 0.f);

        //Array<float> oflow = optical_flow(a, b, max_window_shape);
        //lerr() << oflow;
    }
}

void test_flow_registration() {
    Array<float> moving{
        {1, 2, 3, 4, 5, 6, 7},
        {2, 3, 4, 5, 6, 7, 8},
        {3, 4, 5, 6, 7, 8, 9},
        {4, 5, 6, 7, 8, 9, 10},
        {4, 5, 6, 7, 8, 9, 10},
        {4, 5, 6, 7, 8, 9, 10},};
    Array<float> fixed = moving + 1.2f;
    Array<float> displacement;
    flow_registration(
        moving,
        fixed,
        displacement,
        3,  // window_size
        2,  // box_size
        5,  // max_displacement
        1); // niterations
    assert_allclose(displacement[0], Array<float>{
        {-2.66667f, -4.f, -2.f,   -2.f,   -2.f, -NAN, -NAN},
        {-1.93333f, -2.13333f, -1.46667f, -1.6f, -1.46666f, -1.2f, -1.2f},
        {-1.2f, -1.2f, -1.2f, -1.2f, -1.2f, -1.2f, -1.2f},
        {-1.2f, -1.2f, -1.2f, -1.2f, -1.2f, -1.2f, -1.2f},
        {-1.2f, -1.2f, -1.2f, -1.2f, -1.2f, -1.2f, -1.2f},
        {-NAN, -NAN, -NAN, -NAN, -NAN, -NAN, -NAN}},
        (float)1e-5);
}


int main(int argc, char** argv) {
    test_patch_registration_1d_0();
    test_patch_registration_1d_1();
    test_patch_registration_2d();
    test_flow_registration();
    return 0;
}
