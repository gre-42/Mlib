#pragma once
#include <Mlib/Math/Float_Type.hpp>
#include <random>

namespace Mlib {

template <class TData>
class FastUniformRandomNumberGenerator {
public:
    typedef typename FloatType<TData>::value_type float_type;
    explicit FastUniformRandomNumberGenerator(unsigned int seed, const float_type& low = 0, const float_type& high = 1)
    : e_(seed),
      d_(low, high)
    {}
    TData operator () () {
        return d_(e_);
    }
    void seed(unsigned int seed) {
        e_.seed(seed);
    }
private:
    std::minstd_rand e_;
    std::uniform_real_distribution<typename FloatType<TData>::value_type> d_;
};

template <class TData>
class FastUniformIntRandomNumberGenerator {
public:
    explicit FastUniformIntRandomNumberGenerator(unsigned int seed, const TData& low, const TData& high)
    : e_(seed),
      d_(low, high)
    {}
    TData operator () () {
        return d_(e_);
    }
    void seed(unsigned int seed) {
        e_.seed(seed);
    }
private:
    std::minstd_rand e_;
    std::uniform_int_distribution<TData> d_;
};

template <class TData>
class FastNormalRandomNumberGenerator {
public:
    typedef typename FloatType<TData>::value_type float_type;
    explicit FastNormalRandomNumberGenerator(unsigned int seed, const float_type& mean = 0, const float_type& stddev = 1)
    : e_(seed),
      d_(mean, stddev)
    {}
    TData operator () () {
        return d_(e_);
    }
    void seed(unsigned int seed) {
        e_.seed(seed);
    }
private:
    std::minstd_rand e_;
    std::normal_distribution<typename FloatType<TData>::value_type> d_;
};

template <class TData>
class FastGammaRandomNumberGenerator {
public:
    typedef typename FloatType<TData>::value_type float_type;
    explicit FastGammaRandomNumberGenerator(unsigned int seed, const float_type& alpha, const float_type& beta = 1)
    : e_(seed),
      d_(alpha, beta)
    {}
    TData operator () () {
        return d_(e_);
    }
    void seed(unsigned int seed) {
        e_.seed(seed);
    }
private:
    std::minstd_rand e_;
    std::gamma_distribution<typename FloatType<TData>::value_type> d_;
};

}
