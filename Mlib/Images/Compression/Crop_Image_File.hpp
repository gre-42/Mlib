#pragma once
#include <cstddef>
#include <filesystem>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

void crop_image_file(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    const FixedArray<size_t, 2>& begin,
    const FixedArray<size_t, 2>& end,
    int jpg_quality = 95);

void crop_image_file(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    const FixedArray<size_t, 2>& size,
    int jpg_quality = 95);

}
