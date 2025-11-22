#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <unordered_map>

namespace Mlib {

class StreetBvh;
template <class TPos>
class TriangleList;
template <class TData>
class Array;
template <class TDataX, class TDataY>
class Interp;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <class TPos>
class VertexHeightBinding;
template <typename TData, size_t... tshape>
class FixedArray;

void apply_displacement_map(
    std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>& displacements,
    const StreetBvh& ground_bvh,
    const StreetBvh& air_bvh,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& triangles,
    std::unordered_map<FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>>& vertex_height_bindings,
    const Array<double>& displacementmap,
    double min_displacement,
    double uv_scale,
    const Interp<float, float>& distance_2_z_scale,
    double scale);

}
