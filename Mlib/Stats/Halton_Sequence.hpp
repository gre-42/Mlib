#pragma once
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>

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

MLIB_STATS_API extern double SHUFFLED_HALTON_1K[1'000];
static const size_t SHUFFLED_HALTON_1K_COUNT = sizeof(SHUFFLED_HALTON_1K) / sizeof(SHUFFLED_HALTON_1K[0]);
static const unsigned int HALTON_LUT_B = 2;
static const unsigned int HALTON_SEQUENCE_B = 3;

// From: https://en.wikipedia.org/wiki/Halton_sequence#cite_note-BerblingerSchlier1991-2
template <class TData>
class HaltonSequence {
public:
    HaltonSequence(unsigned int b, const TData& low = 0, const TData& high = 1)
        : b_{ b }
        , low_{ low }
        , high_{ high }
    {
        reset();
    }
    TData operator () () {
        auto x = d_ - n_;
        if (x == 1) {
            n_ = 1;
            d_ *= b_;
        } else {
            auto y = d_ / b_;
            while (x <= y) {
                y /= b_;
            }
            n_ = (b_ + 1) * y - x;
        }
        return (TData(n_) / TData(d_)) * (high_ - low_) + low_;
    }
    void reset() {
        n_ = 0;
        d_ = 1;
    }
private:
    unsigned int n_;
    unsigned int d_;
    unsigned int b_;
    TData low_;
    TData high_;
};

template <class TData>
class PermutedHaltonSequence {
public:
    PermutedHaltonSequence(unsigned int b, const TData& low = 0, const TData& high = 1, size_t buffer_size = 10)
        : h_{ b, low, high }
        , irng_{ 0, 0, buffer_size }
        , buffer_(buffer_size)
    {
        reset();
    }
    TData operator () () {
        size_t i = irng_();
        auto result = buffer_[i];
        buffer_[i] = h_();
        return result;
    }
    void reset() {
        h_.reset();
        irng_.seed(0);
        for (auto& v : buffer_) {
            v = h_();
        }
    }
private:
    HaltonSequence<TData> h_;
    FastUniformIntRandomNumberGenerator<size_t> irng_;
    std::vector<TData> buffer_;
};

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
        return TData(SHUFFLED_HALTON_1K[index_]) * (high_ - low_) + low_;
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
    HybridHaltonSequence(unsigned int seed, const TData& low = 0, const TData& high = 1)
        : lut_{ seed, -0.5, 0.5 }
        , phs_{ HALTON_SEQUENCE_B, -0.5, 0.5 }
        , low_{ low }
        , high_{ high }
    {}
    TData operator () () {
        auto res = lut_() + phs_();
        if (res < 0) {
            res += 1;
        }
        return low_ + res * (high_ - low_);
    }
    void seed(unsigned int seed) {
        lut_.seed(seed);
        phs_.reset();
    }
private:
    PrecomputedHaltonSequence<TData> lut_;
    PermutedHaltonSequence<TData> phs_;
    TData low_;
    TData high_;
};

}
