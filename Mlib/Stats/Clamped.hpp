#pragma once
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib {

template <class TValue, class TMin, class TMax>
auto clamped(const TValue& value, const TMin& min, const TMax& max) {
    return minimum(maximum(value, min), max);
}

}
