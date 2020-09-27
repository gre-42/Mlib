#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> linspace(const TData& from, const TData& to, size_t count) {
    Array<TData> result{ArrayShape{count}};
    if (count == 1) {
        result = (from + to) / 2;
    } else {
        for(size_t i = 0; i < count; ++i) {
            result(i) = (from * (count - i - 1) + to * i) / (count - 1);
        }
    }
    return result;
}

}
