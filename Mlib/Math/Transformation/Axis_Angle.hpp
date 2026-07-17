#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class T, size_t ndim>
struct AxisAngle {
    FixedArray<T, ndim> axis;
    T angle;
    FixedArray<T, ndim> w() const {
        return axis * angle;
    }
};

}
