#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Stats/Cdf.hpp>
#include <Mlib/Stats/Halton_Sequence.hpp>
#include <Mlib/Stats/Histogram.hpp>
#include <Mlib/Stats/Histogram_Matching.hpp>
#include <Mlib/Stats/Incomplete_Beta_Distribution.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Logspace.hpp>
#include <Mlib/Stats/Mean_Variance_Iterator.hpp>
#include <Mlib/Stats/Neighbor_Db.hpp>
#include <Mlib/Stats/Online_Nth_Element.hpp>
#include <Mlib/Stats/Quantile.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Stats/Ransac.hpp>
#include <Mlib/Stats/Robust.hpp>
#include <Mlib/Stats/Sort.hpp>
#include <Mlib/Stats/T_Distribution.hpp>
#include <map>

using namespace Mlib;

void test_median() {
    Array<float> a{ 1, 2, 3, 4, 5, 6 };
    assert_isclose<float>(median(a), 4);
}

void test_mad() {
    Array<float> a{ 1, 2, 3, 4, 5, 6 };
    assert_isclose<float>(mad(a), 2.96736f, (float)1e-3);
}

void test_robust_deviation() {
    Array<float> a{ 1, 2, 3, 4, 5, 6 };
    assert_allclose<float>(
        robust_deviation(a),
        Array<float>{-1.011f, -0.674f, -0.337f, 0, 0.337f, 0.674f},
        (float)1e-3);
}

void test_ransac() {
    Array<float> data{ 1, 2, 30, 2, -10, 2, 3, 2, 7, 8, 9 };
    RansacOptions<float> ro{
        .nelems_small = 3,
        .ncalls = 10,
        .inlier_distance_thresh = 1.f,
        .inlier_count_thresh = 3,
        .seed = 1
    };
    Array<size_t> best_ids = ransac<float>(
        data.length(), // nelems_large
        ro,
        [&](const Array<size_t>& indices)
        {
            // lerr() << data[indices] << " | " << abs(data - mean(data[indices])) << " | " << sum(abs(data - mean(data[indices]))) << " | " << mean(data[indices]);
            return abs(data - mean(data[indices]));
        });
    assert_allequal(best_ids, Array<size_t>{ 1, 3, 5, 6, 7 });
}

void test_sort() {
    {
        Array<float> x{ 9, 8, 7, 6, 5 };
        assert_allclose(sorted(x), Array<float>{5, 6, 7, 8, 9});
    }
    
    {
        Array<float> x{
            {1, 8, NAN, 6, NAN, 5},
            {9, NAN, 2, 3, NAN, 3} };
        assert_allequal(
            nan_sorted(x, 0),
            Array<float>{
                {1, 8, 2, 3, NAN, 3},
                {9, NAN, NAN, 6, NAN, 5}});
    }
}

void test_quantiles() {
    Array<float> x{ 9, 8, 7, 6, 5 };
    assert_allclose(
        quantiles(x, Array<float>{0, 0.2f, 0.8f, 1}),
        Array<float>{5, 6, 8, 9});
    assert_isclose(quantile(x, 0.2f), 6.f);
    assert_isclose(nanquantile(Array<float>({ x, NAN * x }), 0.2f), 6.f);
}

void test_argmin() {
    assert_isequal<size_t>(argmin(Array<float>{3, 5, 2, 1, 3, NAN}), 3);
    assert_isequal<size_t>(argmin(Array<float>{NAN}), SIZE_MAX);
    assert_isequal<size_t>(
        argmin(Array<float>{3, 5, 2, 1, 3, NAN}, 0)(),
        3);
    assert_allequal(
        argmin(Array<float>{
            {3, 5, 2, 1, 3, NAN, NAN},
            { 4, 4, 5, 1, 9, 5, NAN }}, 0),
        Array<size_t>{0, 1, 0, 0, 0, 1, SIZE_MAX});
    assert_allequal(
        argmin(Array<float>{
            {3, 5, 2, 1, 3, NAN, NAN},
            { 4, 4, 1, 5, 9, 5, NAN }}, 1),
        Array<size_t>{3, 2});
}

void test_linspace() {
    assert_allclose(
        linspace(2.f, 4.f, 5),
        Array<float>{2.f, 2.5f, 3.f, 3.5f, 4.f});
}

void test_logspace() {
    assert_allclose(
        logspace(2.f, 4.f, 5),
        Array<float>{100.f, 316.228f, 1000.f, 3162.28f, 10000.f},
        (float)1e-2);
}

void test_histogram() {
    Array<size_t> hist;
    Array<float> bins;
    histogram(Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f}, hist, bins, 6);
    assert_allclose(
        hist.casted<float>(),
        Array<float>{1.f, 1.f, 2.f, 1.f, 1.f, 1.f});
    assert_allclose(
        bins,
        Array<float>{1.f, 2.f, 3.f, 4.f, 5.f, 6.f});
}

void test_cdf() {
    Array<float> ccdf;
    Array<float> bins;
    cdf(Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f}, ccdf, bins, 6);
    assert_allclose(
        ccdf.casted<float>(),
        Array<float>{0.142857f, 0.285714f, 0.571429f, 0.714286f, 0.857143f, 1});
    assert_allclose(
        bins,
        Array<float>{1, 2, 3, 4, 5, 6});
}

void test_histogram_matching() {
    assert_allclose(
        histogram_matching(
            Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f},
            Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f},
            6),
        Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f});
    assert_allclose(
        histogram_matching(
            Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f},
            Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f} * 10.f,
            10),
        Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f} * 10.f);
    assert_allclose(
        histogram_matching<unsigned char, unsigned char, float>(
            Array<unsigned char>{1, 2, 3, 3, 4, 5, 6},
            Array<unsigned char>{1, 2, 3, 3, 4, 5, 6} * (unsigned char)1,
            10).casted<float>(),
        Array<float>{1.f, 2.f, 3.f, 3.f, 4.f, 5.f, 6.f} * 1.f);
}

void test_neighbor_db_1d() {
    NeighborDb1d<float> db1d{ Array<float>{1, 2, 6, 2, 4, 6} };
    assert_allclose(db1d.get_neighbors(2.5, 1).casted<float>(), Array<float>{1.f, 3.f});
    assert_allclose(db1d.get_neighbors(2, 1).casted<float>(), Array<float>{0.f, 1.f, 3.f});
    assert_allclose(db1d.get_neighbors(2, 2).casted<float>(), Array<float>{0.f, 1.f, 3.f, 4.f});
    assert_allclose(db1d.get_neighbors(20, 2).casted<float>(), Array<float>{ArrayShape{ 0 }});
    assert_allclose(db1d.get_neighbors(-20, 2).casted<float>(), Array<float>{ArrayShape{ 0 }});
}

void test_neighbor_db() {
    NeighborDb<float> db{ std::list<Array<float>>({
        Array<float>{5, 7, 2},
        Array<float>{5, 7, 1}}) };
    assert_isequal<size_t>(db.count(Array<float>{5.f, 7.f, 1.f}, 0), 1);
    assert_isequal<size_t>(db.count(Array<float>{5.f, 7.f, 1.f}, 1), 2);
    assert_isequal<size_t>(db.count_slow(Array<float>{5.f, 7.f, 1.f}, 1), 2);
    assert_isequal<size_t>(db.count_slow(Array<float>{5.f, 7.f, 10.f}, 1), 0);
    assert_isequal<size_t>(db.count(Array<float>{5.f, 7.f, 10.f}, 1), 0);
}

void test_mean_variance_iterator() {
    MeanVarianceIterator<float> it;
    it.add(0.5f);
    it.add(1.f);
    assert_isclose(it.mean(), 0.75f);
    assert_isclose(it.var(), 0.125f);
    assert_isclose(it.t(), 3.f, (float)1e-5);
    assert_isclose(it.t(1.f), 0.6f, (float)1e-5);

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
    assert_isclose(incbeta<double>(10, 10, 1), 1.00000, 1e-5);

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

void test_random_number_generators() {
    UniformRandomNumberGenerator<float> rng{ 1, 0.f, 1.f };
    float m = 0;
    for (size_t i = 0; i < 1000; ++i) {
        m += rng();
    }
    assert_isclose(m, 500.f, 1e1f);
}

void test_halton_sequence() {
    // generate_halton_lut(1000, 10);
    // generate_rational_halton_lut(RATIONAL_HALTON_B, HALTON_BLOCK_SEED, 1000, RATIONAL_HALTON_BLOCK_SIZE);
    // {
    //     for (unsigned int seed = 0; seed < 10; ++seed) {
    //         linfo() << "seed " << seed;
    //         HaltonSequence<double> rng{ seed };
    //         for (size_t i = 0; i < 5; ++i) {
    //             linfo() << rng();
    //         }
    //     }
    // }
    // {
    //     linfo() << "precomputed";
    //     PrecomputedHaltonSequence<double> rng{ 50 };
    //     for (size_t i = 0; i < 5; ++i) {
    //         linfo() << rng();
    //     }
    // }
    // {
    //     HybridHaltonSequence<double> rng{ 12, -1.5, 3.5 };
    //     for (size_t i = 0; i < 10; ++i) {
    //         lraw() << rng() << ", ";
    //     }
    //     linfo() << "---";
    //     rng.seed(12);
    //     for (size_t i = 0; i < 10; ++i) {
    //         lraw() << rng() << ", ";
    //     }
    // }
    // {
    //     SeedHaltonSequence<double> rng{ 12 , -1.5, 3.5 };
    //     for (size_t i = 0; i < 100; ++i) {
    //         lraw() << rng() << ", ";
    //     }
    //     linfo() << "---";
    //     rng.seed(12);
    //     for (size_t i = 0; i < 100; ++i) {
    //         lraw() << rng() << ", ";
    //     }
    // }
}

void test_online_nth_largest() {
    OnlineNthSmallest<int, double> nl{ 3 };
    nl.insert(4, 44);
    nl.insert(3, 33);
    nl.insert(5, 55);
    nl.insert(1, 11);
    nl.insert(2, 22);
    nl.insert(7, 77);
    nl.insert(8, 88);
    linfo() << nl.nth();
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    try {
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
        test_cdf();
        test_histogram_matching();
        test_neighbor_db_1d();
        test_neighbor_db();
        test_mean_variance_iterator();
        test_incomplete_beta();
        test_t_cdf();
        test_random_number_generators();
        test_halton_sequence();
        test_online_nth_largest();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
