#pragma once
#include <Mlib/Type_Traits/Any_Signed.hpp>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace Mlib {

template <class T, int64_t num>
constexpr bool fits() {
    return (
        num >= std::numeric_limits<T>::lowest() &&
        num <= std::numeric_limits<T>::max());
}

template <class T, auto num>
constexpr auto get_operand() {
    if constexpr (is_any_signed_v<T>) {
        if constexpr (fits<int8_t, num>()) {
            return (int8_t)num;
        } else if constexpr (fits<int16_t, num>()) {
            return (int16_t)num;
        } else if constexpr (fits<int32_t, num>()) {
            return (int32_t)num;
        } else if constexpr (fits<int64_t, num>()) {
            return (int64_t)num;
        }
    } else {
        if constexpr (fits<uint8_t, num>()) {
            return (uint8_t)num;
        } else if constexpr (fits<uint16_t, num>()) {
            return (uint16_t)num;
        } else if constexpr (fits<uint32_t, num>()) {
            return (uint32_t)num;
        } else if constexpr (fits<uint64_t, num>()) {
            return (uint64_t)num;
        }
    }
}

template <class T, auto num>
constexpr static const auto operand = get_operand<T, num>();

}
