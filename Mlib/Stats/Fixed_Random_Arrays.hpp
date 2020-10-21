#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>

namespace Mlib {

template <class TData, size_t... tsize>
FixedArray<TData, tsize...> fixed_random_uniform_array(unsigned int seed) {
    FixedArray<TData, tsize...> result;
    randomize_array_uniform(result, seed);
    return result;
}

}
