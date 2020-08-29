#pragma once
#include "Coordinates.hpp"
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

inline FixedArray<float, 2> i2a(const FixedArray<size_t, 2>& i) {
    return FixedArray<float, 2>{
        i2a(i(id1)),
        i2a(i(id0))};
}

inline FixedArray<size_t, 2> a2i(const FixedArray<float, 2>& a) {
    return FixedArray<size_t, 2>{
        a2i(a(id1)),
        a2i(a(id0))};
}

inline FixedArray<float, 2> a2fi(const FixedArray<float, 2>& a) {
    return FixedArray<float, 2>{
        a2fi(a(id1)),
        a2fi(a(id0))};
}

inline FixedArray<float, 2> fi2a(const FixedArray<float, 2>& i) {
    return FixedArray<float, 2>{
        fi2a(i(id1)),
        fi2a(i(id0))};
}

}
