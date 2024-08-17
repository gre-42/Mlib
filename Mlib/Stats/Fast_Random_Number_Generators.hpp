#pragma once
#include <Mlib/Math/Float_Type.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <random>
#include <tuple>

namespace Mlib {

template <class TData>
class FastUniformRandomNumberGenerator {
public:
    typedef typename FloatType<TData>::value_type float_type;
    explicit FastUniformRandomNumberGenerator(unsigned int seed, const float_type& low = 0, const float_type& high = 1)
    : e_(seed),
      d_(low, high)
    {
        std::ignore = e_();
    }
    TData operator () () {
        return d_(e_);
    }
    void seed(unsigned int seed) {
        e_.seed(seed);
        std::ignore = e_();
    }
private:
    std::minstd_rand e_;
    std::uniform_real_distribution<typename FloatType<TData>::value_type> d_;
};

// The implementation differs among compilers (e.g. g++ vs. MSVC)
// https://stackoverflow.com/questions/44520973/how-can-i-get-an-implementation-agnostic-version-of-stduniform-int-distributio
// template <class TData>
// class FastUniformIntRandomNumberGenerator {
// public:
//     explicit FastUniformIntRandomNumberGenerator(unsigned int seed, const TData& low, const TData& high)
//     : e_(seed),
//       d_(low, high)
//     {
//         std::ignore = e_();
//     }
//     TData operator () () {
//         return d_(e_);
//     }
//     void seed(unsigned int seed) {
//         e_.seed(seed);
//         std::ignore = e_();
//     }
// private:
//     std::minstd_rand e_;
//     std::uniform_int_distribution<TData> d_;
// };

template <class TData>
class FastUniformIntRandomNumberGenerator {
public:
    explicit FastUniformIntRandomNumberGenerator(unsigned int seed, const TData& low, const TData& high)
    : e_(seed),
      d_(integral_cast<int>(low), double(integral_cast<int>(high)) + 1.)
    {
        std::ignore = e_();
    }
    TData operator () () {
        return (TData)std::floor(d_(e_));
    }
    void seed(unsigned int seed) {
        e_.seed(seed);
        std::ignore = e_();
    }
private:
    std::minstd_rand e_;
    std::uniform_real_distribution<double> d_;
};

template <class TData>
class FastNormalRandomNumberGenerator {
public:
    typedef typename FloatType<TData>::value_type float_type;
    explicit FastNormalRandomNumberGenerator(unsigned int seed, const float_type& mean = 0, const float_type& stddev = 1)
    : e_(seed),
      d_(mean, stddev)
    {
        std::ignore = e_();
    }
    TData operator () () {
        return d_(e_);
    }
    void seed(unsigned int seed) {
        e_.seed(seed);
        std::ignore = e_();
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
    {
        std::ignore = e_();
    }
    TData operator () () {
        return d_(e_);
    }
    void seed(unsigned int seed) {
        e_.seed(seed);
        std::ignore = e_();
    }
private:
    std::minstd_rand e_;
    std::gamma_distribution<typename FloatType<TData>::value_type> d_;
};

}
