#pragma once
#include <type_traits>

namespace Mlib {

template<class Enum>
auto consteval to_underlying(Enum e) {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

}
