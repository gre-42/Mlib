#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class FilterExtension;
enum class TargetShapeMode;

void resize_file(
    const Utf8Path& source,
    const Utf8Path& dest,
    const FixedArray<size_t, 2>& target_size,
    FilterExtension filter_extension,
    TargetShapeMode target_shape_mode,
    int jpg_quality);

}
