#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <typename TData, size_t... tshape>
class FixedArray;
class IIntersectionInfo;

void handle_line_triangle_intersection(const IntersectionScene& c);
void handle_line_triangle_intersection(
    const IntersectionScene& c,
    const IIntersectionInfo& iinfo);

}
