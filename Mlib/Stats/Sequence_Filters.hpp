#pragma once
#include <Mlib/Stats/Clamp_Uniform.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

template <class TRationalSequence, class TFloat>
class FloatSequence {
public:
    using value_type = TFloat;

    FloatSequence(const TRationalSequence& h)
        : h_{ h }
    {}
    TFloat operator () () {
        return h_().template as_float<TFloat>();
    }
private:
    TRationalSequence h_;
};

template <class TSourceSequence>
class UniformSequence {
public:
    using value_type = typename std::remove_reference_t<TSourceSequence>::value_type;

    UniformSequence(const TSourceSequence& s, const value_type& low = 0, const value_type& high = 1)
        : s_{ s }
        , low_{ low }
        , high_{ high }
    {}
    value_type operator () () {
        return clamp_uniform(
            low_ + s_() * (high_ - low_),
            low_,
            high_);
    }
private:
    TSourceSequence s_;
    value_type low_;
    value_type high_;
};

template <class TSourceSequence>
class BlockPermutedSequence {
public:
    using value_type = typename std::remove_reference_t<TSourceSequence>::value_type;

    BlockPermutedSequence(const TSourceSequence& s, unsigned int seed, size_t buffer_size)
        : s_{ s }
        , irng_{ seed, 0, buffer_size - 1 }
        , buffer_(buffer_size)
    {
        if (buffer_size == 0) {
            THROW_OR_ABORT("Buffer size is zero");
        }
        this->seed(seed);
    }
    value_type operator () () {
        size_t i = irng_();
        auto result = buffer_[i];
        buffer_[i] = s_();
        return result;
    }
    void seed(unsigned int seed) {
        irng_.seed(seed);
        for (auto& v : buffer_) {
            v = s_();
        }
    }
private:
    TSourceSequence s_;
    FastUniformIntRandomNumberGenerator<size_t> irng_;
    std::vector<value_type> buffer_;
};

}
