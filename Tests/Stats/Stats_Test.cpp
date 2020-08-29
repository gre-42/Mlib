#include <Mlib/Stats/Histogram.hpp>
#include <Mlib/Stats/Incomplete_Beta_Distribution.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Logspace.hpp>
#include <Mlib/Stats/Mean_Variance_Iterator.hpp>
#include <Mlib/Stats/Neighbor_Db.hpp>
#include <Mlib/Stats/Quantile.hpp>
#include <Mlib/Stats/Ransac.hpp>
#include <Mlib/Stats/Robust.hpp>
#include <Mlib/Stats/Sort.hpp>
#include <Mlib/Stats/T_Distribution.hpp>
#include <fenv.h>
#include <map>

using namespace Mlib;

void test_median() {
    Array<float> a{1, 2, 3, 4, 5, 6};
    assert_isclose<float>(median(a), 4);
}

void test_mad() {
    Array<float> a{1, 2, 3, 4, 5, 6};
    assert_isclose<float>(mad(a), 2.96736, 1e-3);
}

void test_robust_deviation() {
    Array<float> a{1, 2, 3, 4, 5, 6};
    assert_allclose<float>(
        robust_deviation(a),
        Array<float>{-1.011, -0.674, -0.337, 0, 0.337, 0.674},
        1e-3);
}

void test_ransac() {
    Array<float> data{1, 2, 3, 2, 1, 2, 3, 4, 7, 8, 9};
    RansacOptions<float> ro;
    ro.nelems_small = 3;
    ro.ncalls = 10;
    ro.inlier_distance_thresh = 1.f;
    ro.inlier_count_thresh = 3;
    ro.seed = 1;
    Array<size_t> best_ids = ransac<float>(
        data.length(), // nelems_large
        ro,
        [&](const Array<size_t>& indices)
    {
        // std::cerr << data[indices] << " | " << abs(data - mean(data[indices])) << " | " << sum(abs(data - mean(data[indices]))) << " | " << mean(data[indices]) << std::endl;
        return abs(data - mean(data[indices]));
    });
    assert_allclose<float>(best_ids.casted<float>(), Array<float>{1, 2, 3, 5, 6, 7});
}

void test_sort() {
    Array<float> x{9, 8, 7, 6, 5};
    assert_allclose(sorted(x), Array<float>{5, 6, 7, 8, 9});
}

void test_quantiles() {
    Array<float> x{9, 8, 7, 6, 5};
    assert_allclose(
        quantiles(x, Array<float>{0, 0.2, 0.8, 1}),
        Array<float>{5, 6, 8, 9});
    assert_isclose(quantile(x, 0.2), 6.f);
    assert_isclose(nanquantile(Array<float>({x, NAN * x}), 0.2), 6.f);
}

void test_argmin() {
    assert_isclose<float>(argmin(Array<float>{3, 5, 2, 1, 3, NAN}), 3);
    assert_isclose<float>(argmin(Array<float>{NAN}), SIZE_MAX);
    assert_isclose<float>(
        argmin(Array<float>{3, 5, 2, 1, 3, NAN}, 0)(),
        3);
    assert_allequal(
        argmin(Array<float>{
            {3, 5, 2, 1, 3, NAN, NAN},
            {4, 4, 5, 1, 9, 5, NAN}}, 0),
        Array<size_t>{0, 1, 0, 0, 0, 1, SIZE_MAX});
    assert_allequal(
        argmin(Array<float>{
            {3, 5, 2, 1, 3, NAN, NAN},
            {4, 4, 1, 5, 9, 5, NAN}}, 1),
        Array<size_t>{3, 2});
}

void test_linspace() {
    assert_allclose(
        linspace(2.f, 4.f, 5.f),
        Array<float>{2, 2.5, 3, 3.5, 4});
}

void test_logspace() {
    assert_allclose(
        logspace(2.f, 4.f, 5.f),
        Array<float>{100, 316.228, 1000, 3162.28, 10000},
        1e-2);
}

void test_histogram() {
    Array<size_t> hist;
    Array<float> bins;
    histogram(Array<float>{1, 2, 3, 3, 4, 5, 6}, hist, bins);
    assert_allclose(
        hist.casted<float>(),
        Array<float>{1, 0, 1, 0, 2, 1, 0, 1, 0, 1, 0});
    assert_allclose(
        bins,
        Array<float>{1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6});
}

void test_neighbor_db_1d() {
    NeighborDb1d<float> db1d{Array<float>{1, 2, 6, 2, 4, 6}};
    assert_allclose(db1d.get_neighbors(2.5, 1).casted<float>(), Array<float>{1, 3});
    assert_allclose(db1d.get_neighbors(2, 1).casted<float>(), Array<float>{0, 1, 3});
    assert_allclose(db1d.get_neighbors(2, 2).casted<float>(), Array<float>{0, 1, 3, 4});
    assert_allclose(db1d.get_neighbors(20, 2).casted<float>(), Array<float>{ArrayShape{0}});
    assert_allclose(db1d.get_neighbors(-20, 2).casted<float>(), Array<float>{ArrayShape{0}});
}

void test_neighbor_db() {
    NeighborDb<float> db{std::list<Array<float>>({
        Array<float>{5, 7, 2},
        Array<float>{5, 7, 1}})};
    assert_isclose<float>(db.count(Array<float>{5, 7, 1}, 0), 1);
    assert_isclose<float>(db.count(Array<float>{5, 7, 1}, 1), 2);
    assert_isclose<float>(db.count_slow(Array<float>{5, 7, 1}, 1), 2);
    assert_isclose<float>(db.count_slow(Array<float>{5, 7, 10}, 1), 0);
    assert_isclose<float>(db.count(Array<float>{5, 7, 10}, 1), 0);
}

void test_mean_variance_iterator() {
    MeanVarianceIterator<float> it;
    it.add(0.5);
    it.add(1);
    assert_isclose(it.mean(), 0.75f);
    assert_isclose(it.var(), 0.125f);
    assert_isclose(it.t(), 3.f, 1e-5);
    assert_isclose(it.t(1.f), 0.6f, 1e-5);

    // from scipy.stats import ttest_1samp
    // ttest_1samp([1, 2, 3], 0)
    // Ttest_1sampResult(statistic=3.464101615137755, pvalue=0.07417990022744853)

    MeanVarianceIterator<float> it2;
    it2.add(1);
    it2.add(2);
    it2.add(3);
    assert_isclose(it2.t(), 3.464101615137755f);
    assert_isclose(it2.p1() * 2, 0.07417990022744853f);
    assert_isclose(it2.p2(), 0.07417990022744853f);
}

void test_incomplete_beta() {
    // Source: https://github.com/codeplea/incbeta/blob/master/test.c
    assert_isclose(incbeta<double>(10, 10, .1), 0.00000, 1e-5);
    assert_isclose(incbeta<double>(10, 10, .3), 0.03255, 1e-5);
    assert_isclose(incbeta<double>(10, 10, .5), 0.50000, 1e-5);
    assert_isclose(incbeta<double>(10, 10, .7), 0.96744, 1e-5);
    assert_isclose(incbeta<double>(10, 10,  1), 1.00000, 1e-5);

    assert_isclose(incbeta<double>(15, 10, .5), 0.15373, 1e-5);
    assert_isclose(incbeta<double>(15, 10, .6), 0.48908, 1e-5);

    assert_isclose(incbeta<double>(10, 15, .5), 0.84627, 1e-5);
    assert_isclose(incbeta<double>(10, 15, .6), 0.97834, 1e-5);

    assert_isclose(incbeta<double>(20, 20, .4), 0.10206, 1e-5);
    assert_isclose(incbeta<double>(40, 40, .4), 0.03581, 1e-5);
    assert_isclose(incbeta<double>(40, 40, .7), 0.99990, 1e-5);
}

void test_t_cdf() {
    // import scipy.stats.distributions
    // scipy.stats.distributions.t.cdf(3, 4)
    // 0.9800290159641406

    assert_isclose(student_t_cdf<double>(3., 4), 0.980029, 1e-5);
    assert_isclose(student_t_sf<double>(3., 4), 0.019971, 1e-5);
}

int main(int argc, const char** argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_median();
    test_mad();
    test_robust_deviation();
    test_ransac();
    test_sort();
    test_quantiles();
    test_argmin();
    test_linspace();
    test_logspace();
    test_histogram();
    test_neighbor_db_1d();
    test_neighbor_db();
    test_mean_variance_iterator();
    test_incomplete_beta();
    test_t_cdf();
    return 0;
}
