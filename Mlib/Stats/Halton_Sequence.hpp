#pragma once
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <algorithm>
#include <cmath>

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

static unsigned int PRIMES_3[] = {
    2, 3
};

// From: https://en.wikipedia.org/wiki/Halton_sequence#cite_note-BerblingerSchlier1991-2
template <class TData>
class HaltonSequence {
public:
    HaltonSequence(unsigned int seed, const TData& low = 0, const TData& high = 1)
        : low_{low}
        , high_{high}
    {
        this->seed(seed);
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
    void seed(unsigned int seed) {
        b_ = PRIMES_3[seed % (sizeof(PRIMES_3) / sizeof(PRIMES_3[0]))];
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
class PrecomputedHaltonSequence {
public:
    PrecomputedHaltonSequence(unsigned int seed, const TData& low = 0, const TData& high = 1)
        : low_{low}
        , high_{high}
    {
        this->seed(seed);
    }
    TData operator () () {
        index_ = (index_ + 1) % SHUFFLED_HALTON_1K_COUNT;
        return TData(SHUFFLED_HALTON_1K[index_]) * (high_ - low_) + low_;
    }
    void seed(unsigned int seed) {
        index_ = seed;
    }
    const TData& low() const {
        return low_;
    }
    const TData& high() const {
        return high_;
    }
private:
    size_t index_;
    TData low_;
    TData high_;
};

template <class TData>
class HybridHaltonSequence {
public:
    HybridHaltonSequence(unsigned int seed, const TData& low = 0, const TData& high = 1)
        : ph_{seed, low, high}
        , urng_{seed, -TData(1) / SHUFFLED_HALTON_1K_COUNT, TData(1) / SHUFFLED_HALTON_1K_COUNT}
    {
        this->seed(seed);
    }
    TData operator () () {
        return std::clamp(ph_() + urng_(), ph_.low(), ph_.high());
    }
    void seed(unsigned int seed) {
        ph_.seed(seed);
        urng_.seed(seed);
    }
private:
    PrecomputedHaltonSequence<TData> ph_;
    FastUniformRandomNumberGenerator<TData> urng_;
};

}
