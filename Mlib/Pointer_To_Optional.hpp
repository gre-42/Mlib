#pragma once
#include <optional>

namespace Mlib {

template <class T>
inline std::optional<T> pointer_to_optional(const T* v) {
    if (v == nullptr) {
        return std::nullopt;
    }
    return *v;
}

}
