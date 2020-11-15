#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TData, size_t tsize>
struct VectorAtPosition {
    FixedArray<TData, tsize> vector;
    FixedArray<TData, tsize> position;
};

}
