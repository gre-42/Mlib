#pragma once

namespace Mlib {

template <class TData>
TData clamp_uniform(const TData& value, const TData& low, const TData& high) {
    if ((value < low) || (value >= high)) {
        return low;
    }
    return value;
}

}
