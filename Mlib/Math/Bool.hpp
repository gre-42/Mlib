#pragma once

namespace Mlib {

template <class TDerived, class TData>
class BaseDenseArray;

template <class TDerived>
inline TDerived& operator |= (BaseDenseArray<TDerived, bool>& a, const BaseDenseArray<TDerived, bool>& b) {
    auto af = a->flat_begin();
    auto bf = b->flat_begin();
    while (af != a->flat_end()) {
        *af++ |= *bf++;
    }
    return *a;
}

template <class TDerived>
inline TDerived operator | (const BaseDenseArray<TDerived, bool>& a, const BaseDenseArray<TDerived, bool>& b) {
    return a->template array_array_binop(*b, [](bool x, bool y){return x | y;});
}

}
