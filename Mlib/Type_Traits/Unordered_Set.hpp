#pragma once
#include <concepts>
#include <type_traits>
#include <unordered_set>

namespace Mlib {

template <typename T>
struct is_std_unordered_set : std::false_type {};

template <typename T, typename Allocator>
struct is_std_unordered_set<std::unordered_set<T, Allocator>> : std::true_type {};

template <typename T>
concept StdUnorderedSet = is_std_unordered_set<std::remove_cvref_t<T>>::value;

}
