#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <memory>

namespace Mlib {

class StreetBvh;
template <class TPos>
class TriangleList;
template <class TData>
class Array;
template <class TDataX, class TDataY>
class Interp;

void apply_displacement_map(
    const StreetBvh& ground_bvh,
    const StreetBvh& air_bvh,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& triangles,
    const Array<double>& displacementmap,
    double min_displacement,
    double uv_scale,
    const Interp<float, float>& distance_2_z_scale,
    double scale);

}
