#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;

template <typename TData, size_t... tshape>
class FixedArray;

namespace CW {

// nelements
template <class T>
size_t nelements(const Array<T>& a) {
    return a.nelements();
}

template <class T, size_t... tshape>
size_t nelements(const FixedArray<T, tshape...>&) {
    return FixedArray<T, tshape...>::nelements();
}

// length
template <class T>
size_t length(const Array<T>& a) {
    return a.length();
}

template <class T, size_t... tshape>
size_t length(const FixedArray<T, tshape...>&) {
    return FixedArray<T, tshape...>::length();
}

}

}
