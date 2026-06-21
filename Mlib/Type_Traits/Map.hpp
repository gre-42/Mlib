#pragma once
#include <concepts>
#include <map>
#include <type_traits>

namespace Mlib {

template <typename T>
struct is_std_map : std::false_type {};

template <typename Key, typename T, typename Compare, typename Allocator>
struct is_std_map<std::map<Key, T, Compare, Allocator>> : std::true_type {};

template <typename T>
concept StdMap = is_std_map<std::remove_cvref_t<T>>::value;

}
