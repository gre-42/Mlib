#pragma once
#ifndef __MINGW32__

#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct ColoredVertex;

void mesh_subtract(std::list<FixedArray<ColoredVertex, 3>>& a, const std::list<FixedArray<ColoredVertex, 3>>& b);

}

#endif
