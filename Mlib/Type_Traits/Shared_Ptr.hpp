#pragma once
#include <memory>

namespace Mlib {

template <typename T>
struct is_shared_ptr : std::false_type {};

template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template <typename T>
concept SharedPtr = is_shared_ptr<std::remove_cvref_t<T>>::value;

template <typename T>
concept NoSharedPtr = !SharedPtr<T>;

}
