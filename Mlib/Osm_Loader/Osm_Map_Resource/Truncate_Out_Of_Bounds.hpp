#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

struct NodesAndWays;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;

void truncate_out_of_bounds(
    NodesAndWays& naws,
    const AxisAlignedBoundingBox<CompressedScenePos, 2>& aabb);

}
