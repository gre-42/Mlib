#pragma once
#include <concepts>

namespace Mlib {

template <std::unsigned_integral T>
static auto minus_modulo(T a, T b) {
    // Deduce the signed counterpart (e.g., uint16_t -> int16_t)
    using SignedType = std::make_signed_t<T>;
    return SignedType(a - b);
}

template <std::unsigned_integral T>
bool is_newer(T incoming, T newest) {
    return minus_modulo(incoming, newest) > 0;
}

}
