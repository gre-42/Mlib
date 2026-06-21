#pragma once
#include <concepts>
#include <list>
#include <type_traits>

namespace Mlib {

template <typename T>
struct is_std_list : std::false_type {};

template <typename T, typename Allocator>
struct is_std_list<std::list<T, Allocator>> : std::true_type {};

template <typename T>
concept StdList = is_std_list<std::remove_cvref_t<T>>::value;

}
