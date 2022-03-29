#pragma once

namespace Mlib {

class ResourceNameCycle;
class BatchResourceInstantiator;
class TriangleList;

void add_grass_inside_triangles(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const TriangleList& triangles,
    float scale,
    float distance);

}
