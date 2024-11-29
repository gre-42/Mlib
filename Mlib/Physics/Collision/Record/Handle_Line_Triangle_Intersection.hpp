#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <typename TData, size_t... tshape>
class FixedArray;
struct IntersectionInfo;

void handle_line_triangle_intersection(const IntersectionScene& c);
void handle_line_triangle_intersection(
    const IntersectionScene& c,
    const IntersectionInfo& iinfo);

}
