#pragma once
#include <concepts>
#include <string>
#include <type_traits>

namespace Mlib {

template <typename T>
struct is_std_basic_string : std::false_type {};

template <typename T, typename Traits, typename Allocator>
struct is_std_basic_string<std::basic_string<T, Traits, Allocator>> : std::true_type {};

template <typename T>
concept StdBasicString = is_std_basic_string<std::remove_cvref_t<T>>::value;

}
