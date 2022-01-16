#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;

template <class TData, size_t tsize>
struct VectorAtPosition {
    FixedArray<TData, tsize> vector;
    FixedArray<TData, tsize> position;
    VectorAtPosition transformed(const TransformationMatrix<TData, tsize>& m) const {
        return VectorAtPosition{
            .vector = m.rotate(vector),
            .position = m.transform(position)};
    }
};

}
