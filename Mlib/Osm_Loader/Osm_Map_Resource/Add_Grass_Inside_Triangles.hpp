#pragma once
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

class ResourceNameCycle;
class BatchResourceInstantiator;
template <class TPos>
class TriangleList;

void add_grass_inside_triangles(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const TriangleList<CompressedScenePos>& triangles,
    float scale,
    CompressedScenePos distance);

}
