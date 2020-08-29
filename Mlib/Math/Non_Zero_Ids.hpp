#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Base_Dense_Array.hpp>

namespace Mlib {

template <class TDerived, class TData>
Array<size_t> nonzero_ids(const BaseDenseArray<TDerived, TData>& a) {
    size_t len = 0;
    Array<size_t> result{ArrayShape{a->nelements()}.appended(a->ndim())};
    a->shape().foreach([&](const ArrayShape& s){
        if ((*a)(s) != TData(0)) {
            result[len++] = Array<size_t>::from_shape(s);
        }
    });
    return result.reshaped(ArrayShape{len}.appended(a->ndim()));
}

}
