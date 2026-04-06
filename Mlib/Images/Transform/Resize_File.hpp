#pragma once
#include <cstddef>
#include <filesystem>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class FilterExtension;
enum class TargetShapeMode;

void resize_file(
    const std::filesystem::path& source,
    const std::filesystem::path& dest,
    const FixedArray<size_t, 2>& target_size,
    FilterExtension filter_extension,
    TargetShapeMode target_shape_mode,
    int jpg_quality);

}
