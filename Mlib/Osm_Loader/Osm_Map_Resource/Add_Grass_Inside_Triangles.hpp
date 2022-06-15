#pragma once

namespace Mlib {

class ResourceNameCycle;
class BatchResourceInstantiator;
template <class TPos>
class TriangleList;

void add_grass_inside_triangles(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const TriangleList<double>& triangles,
    float scale,
    float distance);

}
