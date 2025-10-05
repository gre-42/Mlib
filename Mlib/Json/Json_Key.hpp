#pragma once
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace Mlib {

template <class T>
concept JsonKey =
    std::is_convertible_v<T, std::string_view> ||
    std::is_convertible_v<T, std::vector<std::string>>;

}
