#pragma once
#include <concepts>
#include <type_traits>

namespace Mlib {

template <class T>
concept Enum = std::is_enum_v<T>;

}
