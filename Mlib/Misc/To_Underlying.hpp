#pragma once
#include <type_traits>

namespace Mlib {

template<class Enum>
auto constexpr to_underlying(Enum e) {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

}
