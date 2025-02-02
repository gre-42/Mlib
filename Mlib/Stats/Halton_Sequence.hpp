#pragma once
#include <Mlib/Math/Rational_Number.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Stats/Sequence_Filters.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifdef _MSC_VER
#ifdef MlibStats_EXPORTS
#define MLIB_STATS_API __declspec(dllexport)
#else
#define MLIB_STATS_API __declspec(dllimport)
#endif
#else
#define MLIB_STATS_API
#endif

namespace Mlib {

void generate_halton_lut(size_t nnumbers, size_t block_size);
void generate_rational_halton_lut(unsigned int b, unsigned int seed, size_t nnumbers, size_t block_size);

MLIB_STATS_API extern double SHUFFLED_HALTON_1K[1'000];
MLIB_STATS_API extern RationalNumber<unsigned int> SHUFFLED_RATIONAL_HALTON_1K[1'000];

static const size_t SHUFFLED_HALTON_1K_COUNT = sizeof(SHUFFLED_HALTON_1K) / sizeof(SHUFFLED_HALTON_1K[0]);
static const unsigned int HALTON_LUT_B = 2;
static const unsigned int HALTON_SEQUENCE_B = 3;

static const unsigned int RATIONAL_HALTON_B = 3;
static const size_t RATIONAL_HALTON_BLOCK_SIZE = 10;
static const size_t RATIONAL_HALTON_1K_COUNT = sizeof(SHUFFLED_RATIONAL_HALTON_1K) / sizeof(SHUFFLED_RATIONAL_HALTON_1K[0]);
static const unsigned int HALTON_BLOCK_SEED = 23421;

static const RationalNumber<unsigned int> DEFAULT_RATIONAL_HALTON_SEED = { 0, 1 };

// From: https://en.wikipedia.org/wiki/Halton_sequence#cite_note-BerblingerSchlier1991-2
class RationalHaltonSequence {
public:
    using value_type = RationalNumber<unsigned int>;

    explicit RationalHaltonSequence(RationalNumber<unsigned int> seed, unsigned int b)
        : b_{ b }
    {
        this->seed(seed);
    }
    RationalNumber<unsigned int> operator () () {
        auto x = r_.d - r_.n;
        if (x == 1) {
            r_.n = 1;
            r_.d *= b_;
        } else {
            auto y = r_.d / b_;
            while (x <= y) {
                y /= b_;
            }
            r_.n = (b_ + 1) * y - x;
        }
        return r_;
    }
    void seed(RationalNumber<unsigned int> seed) {
        r_ = seed;
    }
private:
    RationalNumber<unsigned int> r_;
    unsigned int b_;
};

template <class T>
using HaltonSequence = FloatSequence<RationalHaltonSequence, T>;

template <class T>
using PermutedRationalHaltonSequence = BlockPermutedSequence<RationalHaltonSequence>;

template <class T>
using PermutedHaltonSequence = BlockPermutedSequence<FloatSequence<RationalHaltonSequence, T>>;

template <class TData>
class PrecomputedHaltonSequence {
public:
    PrecomputedHaltonSequence(unsigned int seed, const TData& low = 0, const TData& high = 1)
        : low_{low}
        , high_{high}
    {
        this->seed(seed);
    }
    TData operator () () {
        ++ncalls_;
        index_ = (index_ + 1) % SHUFFLED_HALTON_1K_COUNT;
        return clamp_uniform(
            TData(SHUFFLED_HALTON_1K[index_]) * (high_ - low_) + low_,
            low_,
            high_);
    }
    void seed(unsigned int seed) {
        index_ = seed;
        ncalls_ = 0;
    }
    const TData& low() const {
        return low_;
    }
    const TData& high() const {
        return high_;
    }
    unsigned int ncalls() const {
        return ncalls_;
    }
private:
    size_t index_;
    unsigned int ncalls_;
    TData low_;
    TData high_;
};

template <class TData>
class HybridHaltonSequence {
public:
    HybridHaltonSequence(unsigned int seed, const TData& low = 0, const TData& high = 1, size_t buffer_size = 10)
        : lut_{ seed, -0.5, 0.5 }
        , rhs_{ DEFAULT_RATIONAL_HALTON_SEED, HALTON_SEQUENCE_B }
        , phs_{ rhs_, seed, buffer_size }
        , uphs_{ phs_, -0.5, 0.5 }
        , low_{ low }
        , high_{ high }
    {}
    TData operator () () {
        auto res = lut_() + uphs_();
        if (res < 0) {
            res += 1;
        }
        return clamp_uniform(
            low_ + res * (high_ - low_),
            low_,
            high_);
    }
    void seed(unsigned int seed) {
        lut_.seed(seed);
        rhs_.seed(DEFAULT_RATIONAL_HALTON_SEED);
        phs_.seed(seed);
    }
private:
    using Phs = BlockPermutedSequence<FloatSequence<RationalHaltonSequence&, TData>>;
    PrecomputedHaltonSequence<TData> lut_;
    RationalHaltonSequence rhs_;
    Phs phs_;
    UniformSequence<Phs&> uphs_;
    TData low_;
    TData high_;
};

template <class TData>
class SeedHaltonSequence {
public:
    SeedHaltonSequence(unsigned int seed, const TData& low = 0, const TData& high = 1)
        : r_{ SHUFFLED_RATIONAL_HALTON_1K[seed % RATIONAL_HALTON_1K_COUNT], RATIONAL_HALTON_B }
        , h_{ {r_, low, high }, seed, RATIONAL_HALTON_BLOCK_SIZE }
    {}
    TData operator () () {
        return h_();
    }
    void seed(unsigned int seed) {
        r_.seed(SHUFFLED_RATIONAL_HALTON_1K[seed % RATIONAL_HALTON_1K_COUNT]);
        h_.seed(seed);
    }
private:
    RationalHaltonSequence r_;
    BlockPermutedSequence<UniformSequence<FloatSequence<RationalHaltonSequence&, TData>>> h_;
};

}
