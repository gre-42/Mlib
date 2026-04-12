#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

void crop_image_file(
    const Utf8Path& source,
    const Utf8Path& destination,
    const FixedArray<size_t, 2>& begin,
    const FixedArray<size_t, 2>& end,
    int jpg_quality = 95);

void crop_image_file(
    const Utf8Path& source,
    const Utf8Path& destination,
    const FixedArray<size_t, 2>& size,
    int jpg_quality = 95);

}
