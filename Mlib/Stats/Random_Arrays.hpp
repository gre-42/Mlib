#pragma once
#include <Mlib/Stats/Random_Number_Generators.hpp>

namespace Mlib {

/*
 * Uniformly random array
 */
template <class TDerived, class TData>
void randomize_array_uniform(BaseDenseArray<TDerived, TData>& a, unsigned int seed) {
    UniformRandomNumberGenerator<TData> r{seed};
    for(TData& v : a->flat_iterable()) {
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
template <class TDerived, class TData>
void randomize_array_normal(BaseDenseArray<TDerived, TData>& a, unsigned int seed) {
    NormalRandomNumberGenerator<TData> r{seed};
    for(TData& v : a->flat_iterable()) {
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
