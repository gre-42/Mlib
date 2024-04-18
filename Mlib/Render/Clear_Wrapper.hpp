#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

enum class ClearBackend {
    SHADER,
    AUTO
};

void clear_color(
    const FixedArray<float, 4>& color,
    ClearBackend backend = ClearBackend::AUTO);
void clear_depth(
    ClearBackend backend = ClearBackend::AUTO);
void clear_color_and_depth(
    const FixedArray<float, 4>& color,
    ClearBackend backend = ClearBackend::AUTO);

class ClearWrapperGuard {
public:
    ClearWrapperGuard();
    ~ClearWrapperGuard();
};

}
