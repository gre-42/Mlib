#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Images/Target_Shape_Mode.hpp>
#include <stdexcept>

namespace Mlib {

inline size_t get_target_shape(
    size_t source,
    size_t dest,
    TargetShapeMode mode)
{
    switch (mode) {
    case TargetShapeMode::DEST:
        return dest;
    case TargetShapeMode::SOURCE_WHEN_ZERO:
        return dest == 0 ? source : dest;
    }
    throw std::runtime_error("Unknown target shape mode");
}

inline ArrayShape get_target_shape(
    const ArrayShape& source,
    const ArrayShape& dest,
    TargetShapeMode mode)
{
    return arrayshape_arrayshape_applied(
        source,
        dest,
        [mode](size_t s, size_t d){ return get_target_shape(s, d, mode); });
}

template <size_t tndim>
FixedArray<size_t, tndim> get_target_shape(
    const FixedArray<size_t, tndim>& source,
    const FixedArray<size_t, tndim>& dest,
    TargetShapeMode mode)
{
    return source.array_array_binop(
        source,
        dest,
        [mode](size_t s, size_t d){ return get_target_shape(s, d, mode); });
}

}
