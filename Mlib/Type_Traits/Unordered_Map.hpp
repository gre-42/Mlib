#pragma once
#include <concepts>
#include <type_traits>
#include <unordered_map>

namespace Mlib {

template <typename T>
struct is_std_unordered_map : std::false_type {};

template <typename Key, typename T, typename Compare, typename Allocator>
struct is_std_unordered_map<std::unordered_map<Key, T, Compare, Allocator>> : std::true_type {};

template <typename T>
concept StdUnorderedMap = is_std_unordered_map<std::remove_cvref_t<T>>::value;

}
