#pragma once
#include <concepts>
#include <optional>
#include <type_traits>

namespace Mlib {

template <typename T>
struct is_std_optional : std::false_type {};

template <typename T>
struct is_std_optional<std::optional<T>> : std::true_type {};

template <typename T>
concept StdOptional = is_std_optional<std::remove_cvref_t<T>>::value;

}
