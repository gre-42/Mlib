#pragma once
#include <memory>

namespace Mlib {

template <typename T>
struct is_unique_ptr : std::false_type {};

template <typename T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

template <typename T>
concept UniquePtr = is_unique_ptr<std::remove_cvref_t<T>>::value;

template <typename T>
concept NoUniquePtr = !UniquePtr<T>;

}
