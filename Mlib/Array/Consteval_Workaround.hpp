#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;

template <class TData>
class ConjugateTransposeArray;

template <typename TData, size_t... tshape>
class FixedArray;

namespace CW {

// nelements
template <class T>
size_t nelements(const Array<T>& a) {
    return a.nelements();
}

template <class T, size_t... tshape>
constexpr size_t nelements(const FixedArray<T, tshape...>&) {
    return FixedArray<T, tshape...>::nelements();
}

// length
template <class T>
size_t length(const Array<T>& a) {
    return a.length();
}

template <class T, size_t... tshape>
constexpr size_t length(const FixedArray<T, tshape...>&) {
    return FixedArray<T, tshape...>::length();
}

// ndim
template <class T>
size_t ndim(const Array<T>& a) {
    return a.ndim();
}

template <class T>
size_t ndim(const ConjugateTransposeArray<T>& a) {
    return a.ndim();
}

template <class T, size_t... tshape>
constexpr size_t ndim(const FixedArray<T, tshape...>&) {
    return FixedArray<T, tshape...>::ndim();
}

// static_shape
template <size_t N, class T>
size_t static_shape(const Array<T>& a) {
    return a.shape(N);
}

template <size_t N, class T>
size_t static_shape(const ConjugateTransposeArray<T>& a) {
    return a.shape(N);
}

template <size_t N, class T, size_t... tshape>
constexpr size_t static_shape(const FixedArray<T, tshape...>&) {
    return FixedArray<T, tshape...>::template static_shape<N>();
}

}

}
