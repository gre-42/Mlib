#pragma once
#if !defined(__MINGW32__) && !defined(_MSC_VER)

#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct ColoredVertex;

void mesh_subtract(std::list<FixedArray<ColoredVertex, 3>>& a, const std::list<FixedArray<ColoredVertex, 3>>& b);

}

#endif
