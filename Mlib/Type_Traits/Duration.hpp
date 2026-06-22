#pragma once
#include <chrono>
#include <type_traits>

namespace Mlib {

template <typename T>
struct is_chrono_duration : std::false_type {};

template <typename Rep, typename Period>
struct is_chrono_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

template <typename T>
concept ChronoDuration = is_chrono_duration<std::remove_cvref_t<T>>::value;

}
