#pragma once
#include <concepts>
#include <set>
#include <type_traits>

namespace Mlib {

template <typename T>
struct is_std_set : std::false_type {};

template <typename T, typename Allocator>
struct is_std_set<std::set<T, Allocator>> : std::true_type {};

template <typename T>
concept StdSet = is_std_set<std::remove_cvref_t<T>>::value;

}
