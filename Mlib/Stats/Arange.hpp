#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template <class TData>
Array<TData> arange(size_t length) {
    Array<TData> a{ArrayShape{length}};
    for(size_t i = 0; i < length; ++i) {
        a(i) = i;
    }
    return a;
}

template <class TData, class TInt>
Array<TData> arange(TInt start, TInt stop) {
    Array<TData> a{ArrayShape{stop - start}};
    for(TInt i = 0; i < stop - start; ++i) {
        a(i) = start + i;
    }
    return a;
}

}
