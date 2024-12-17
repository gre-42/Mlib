#pragma once
#include <type_traits>

namespace Mlib {

// From: https://en.cppreference.com/w/cpp/types/is_signed
namespace detail
{
    template<typename T>
    struct is_any_signed : std::integral_constant<bool, T(-1.f) < T(0.f)> {};
}

template<typename T>
struct is_any_signed : detail::is_any_signed<T>::type {};

template<typename T>
static const bool is_any_signed_v = is_any_signed<T>{};

}
