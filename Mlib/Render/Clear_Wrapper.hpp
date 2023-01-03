#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

void clear_color(const FixedArray<float, 4>& color);
void clear_depth();
void clear_color_and_depth(const FixedArray<float, 4>& color);

class ClearWrapperGuard {
public:
    ClearWrapperGuard();
    ~ClearWrapperGuard();
};

}
