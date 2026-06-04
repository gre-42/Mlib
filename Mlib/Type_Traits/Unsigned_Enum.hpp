#pragma once
#include <concepts>
#include <type_traits>

namespace Mlib {

template <typename T>
concept UnsignedEnum = std::is_enum_v<T> && 
                       std::unsigned_integral<std::underlying_type_t<T>>;

}
