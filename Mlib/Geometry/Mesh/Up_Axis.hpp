#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

enum class UpAxis {
    Y = 1,
    Z = 2
};

UpAxis up_axis_from_string(const std::string& s);

template <class T>
FixedArray<T, 2> non_up_axis(const FixedArray<T, 3>& v, UpAxis up_axis) {
    if (up_axis == UpAxis::Y) {
        return FixedArray<T, 2>{v(0), v(2)};
    } else if (up_axis == UpAxis::Z) {
        return FixedArray<T, 2>{v(0), v(1)};
    } else {
        THROW_OR_ABORT("Unknown up axis");
    }
}

}
