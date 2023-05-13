#pragma once
#include <type_traits>

namespace Mlib {

template <class TOut, class TIn>
TOut optional_cast(TIn in) {
    if constexpr (std::is_convertible_v<TIn, TOut>) {
        return in;
    } else {
        return nullptr;
    }
}

}
