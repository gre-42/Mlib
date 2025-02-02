#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

enum class ClearBackend {
    SHADER,
    AUTO
};

// Defaulting to "ClearBackend::SHADER" because glClear does
// not respect the viewport settings.
void clear_color(
    const FixedArray<float, 4>& color,
    ClearBackend backend = ClearBackend::SHADER);

// Defaulting to "ClearBackend::SHADER" because glClear does
// not respect the viewport settings.
void clear_depth(
    ClearBackend backend = ClearBackend::SHADER);

// Defaulting to "ClearBackend::SHADER" because glClear does
// not respect the viewport settings.
void clear_color_and_depth(
    const FixedArray<float, 4>& color,
    ClearBackend backend = ClearBackend::SHADER);

class ClearWrapperGuard {
public:
    ClearWrapperGuard();
    ~ClearWrapperGuard();
};

}
