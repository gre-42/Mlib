#pragma once
#include <optional>

namespace Mlib {

template <class T>
T* try_get_value(std::optional<T>& o) {
    if (!o.has_value()) {
        return nullptr;
    }
    return &o.value();
}

template <class T>
const T* try_get_value(const std::optional<T>& o) {
    if (!o.has_value()) {
        return nullptr;
    }
    return &o.value();
}

}
