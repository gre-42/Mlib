#pragma once
#include <cmath>

namespace Mlib {

static double SHUFFLED_HALTON_100[] = {
    0.671875, 0.882812, 0.375, 0.859375, 0.546875, 0.0390625, 0.453125, 0.78125, 0.953125,
    0.484375, 0.03125, 0.015625, 0.601562, 0.609375, 0.515625, 0.796875, 0.445312, 0.421875,
    0.320312, 0.40625, 0.132812, 0.125, 0.570312, 0.640625, 0.664062, 0.109375, 0.171875,
    0.1875, 0.59375, 0.101562, 0.476562, 0.3125, 0.148438, 0.390625, 0.203125, 0.84375,
    0.0234375, 0.539062, 0.914062, 0.5625, 0.164062, 0.90625, 0.890625, 0.296875, 0.632812,
    0.234375, 0.4375, 0.765625, 0.289062, 0.875, 0.28125, 0.382812, 0.851562, 0.75, 0.34375,
    0.726562, 0.984375, 0.828125, 0.703125, 0.71875, 0.21875, 0.0078125, 0.96875, 0.523438,
    0.15625, 0.359375, 0.9375, 0.53125, 0.25, 0.6875, 0.507812, 0.46875, 0.226562, 0.414062,
    0.976562, 0.0703125, 0.078125, 0.773438, 0.5, 0.8125, 0.820312, 0.921875, 0.195312,
    0.273438, 0.757812, 0.695312, 0.09375, 0.265625, 0.945312, 0.140625, 0.328125, 0.257812,
    0.046875, 0.578125, 0.789062, 0.734375, 0.65625, 0.625, 0.351562, 0.0625
};

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
        index_ = (index_ + 1) % (sizeof(SHUFFLED_HALTON_100) / sizeof(SHUFFLED_HALTON_100[0]));
        return TData(SHUFFLED_HALTON_100[index_]) * (high_ - low_) + low_;
    }
    void seed(unsigned int seed) {
        index_ = seed;
    }
private:
    size_t index_;
    TData low_;
    TData high_;
};

}
