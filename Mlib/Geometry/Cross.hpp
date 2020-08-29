#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template <class TData>
Array<TData> cross(const Array<TData>& k) {
    assert(k.length() == 3);
    return Array<TData>{
        {0, -k(2), k(1)},
        {k(2), 0, -k(0)},
        {-k(1), k(0), 0}};
}

template <class TData>
Array<TData> cross(const Array<TData>& a, const Array<TData>& b) {
    assert(a.length() == 3);
    assert(b.length() == 3);
    return Array<TData>{
        -a(2) * b(1) + a(1) * b(2),
        a(2) * b(0) -a(0) * b(2),
        -a(1) * b(0) + a(0) * b(1)};
}

}
