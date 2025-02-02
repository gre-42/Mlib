#pragma once
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <complex>

namespace Mlib {

/*
 * Uniformly random array
 */
template <class TFloat>
Array<std::complex<TFloat>> uniform_random_complex_array(const ArrayShape& shape, unsigned int seed) {
    auto ar = uniform_random_array<std::complex<TFloat>>(shape, seed);
    auto ai = uniform_random_array<std::complex<TFloat>>(shape, seed + 1);
    return ar + std::complex<TFloat>(0, 1) * ai;
}

template <class TDerived, class TData>
void randomize_array_uniform(BaseDenseArray<TDerived, TData>& a, unsigned int seed) {
    UniformRandomNumberGenerator<TData> r{seed};
    for (TData& v : a->flat_iterable()) {
        v = r();
    }
}

template <class TData>
Array<TData> uniform_random_array(const ArrayShape& shape, unsigned int seed) {
    Array<TData> a(shape);
    randomize_array_uniform(a, seed);
    return a;
}

/*
 * Normal random array
 */
template <class TFloat>
Array<std::complex<TFloat>> normal_random_complex_array(const ArrayShape& shape, unsigned int seed) {
    auto ar = normal_random_array<std::complex<TFloat>>(shape, seed);
    auto ai = normal_random_array<std::complex<TFloat>>(shape, seed + 1);
    return ar + std::complex<TFloat>(0, 1) * ai;
}

template <class TDerived, class TData>
void randomize_array_normal(BaseDenseArray<TDerived, TData>& a, unsigned int seed) {
    NormalRandomNumberGenerator<TData> r{seed};
    for (TData& v : a->flat_iterable()) {
        v = r();
    }
}

template <class TData>
Array<TData> normal_random_array(const ArrayShape& shape, unsigned int seed) {
    Array<TData> a(shape);
    randomize_array_uniform(a, seed);
    return a;
}

}
