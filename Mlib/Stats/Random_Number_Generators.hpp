#pragma once
#include <Mlib/Math/Float_Type.hpp>
#include <complex>
#include <random>

namespace Mlib {

template <class TData>
class UniformRandomNumberGenerator {
public:
    typedef typename FloatType<TData>::value_type float_type;
    explicit UniformRandomNumberGenerator(unsigned int seed, const float_type& low = 0, const float_type& high = 1)
    : e_(seed),
      d_(low, high)
    {
        assert(seed != 0); // seed 0 is identical to seed 1
        e_(); // skip first element, it is close to zero
    }
    TData operator () () {
        return d_(e_);
    }
private:
    std::default_random_engine e_;
    std::uniform_real_distribution<typename FloatType<TData>::value_type> d_;
};

template <class TData>
class UniformIntRandomNumberGenerator {
public:
    explicit UniformIntRandomNumberGenerator(unsigned int seed, const TData& low, const TData& high)
    : e_(seed),
      d_(low, high)
    {
        assert(seed != 0); // seed 0 is identical to seed 1
        e_(); // skip first element, it is close to zero
    }
    TData operator () () {
        return d_(e_);
    }
private:
    std::default_random_engine e_;
    std::uniform_int_distribution<TData> d_;
};

template <class TData>
class NormalRandomNumberGenerator {
public:
    typedef typename FloatType<TData>::value_type float_type;
    explicit NormalRandomNumberGenerator(unsigned int seed, const float_type& mean = 0, const float_type& stddev = 1)
    : e_(seed),
      d_(mean, stddev)
    {
        e_(); // skip first element, it is close to zero
    }
    TData operator () () {
        return d_(e_);
    }
private:
    std::default_random_engine e_;
    std::normal_distribution<typename FloatType<TData>::value_type> d_;
};

template <class TData>
class GammaRandomNumberGenerator {
public:
    typedef typename FloatType<TData>::value_type float_type;
    explicit GammaRandomNumberGenerator(unsigned int seed, const float_type& alpha, const float_type& beta = 1)
    : e_(seed),
      d_(alpha, beta)
    {
        e_(); // skip first element, it is close to zero
    }
    TData operator () () {
        return d_(e_);
    }
private:
    std::default_random_engine e_;
    std::gamma_distribution<typename FloatType<TData>::value_type> d_;
};

}
