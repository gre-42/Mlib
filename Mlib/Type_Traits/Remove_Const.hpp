#pragma once
#include <type_traits>

namespace Mlib {

template <class T>
inline T& remove_const(const T& v) {
    return const_cast<T&>(v);
}

template <class T>
inline T* remove_const(const T* v) {
    return const_cast<T*>(v);
}

}
