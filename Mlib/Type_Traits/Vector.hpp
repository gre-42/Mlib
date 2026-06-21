#pragma once
#include <concepts>
#include <type_traits>
#include <vector>

namespace Mlib {

template <typename T>
struct is_std_vector : std::false_type {};

template <typename T, typename Allocator>
struct is_std_vector<std::vector<T, Allocator>> : std::true_type {};

template <typename T>
concept StdVector = is_std_vector<std::remove_cvref_t<T>>::value;

}
