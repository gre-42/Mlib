#include "Apply_Displacement_Map.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>

using namespace Mlib;

void Mlib::apply_displacement_map(
    const StreetBvh& ground_bvh,
    const std::list<std::shared_ptr<TriangleList<double>>>& triangles,
    const Array<double>& displacementmap,
    double uv_scale,
    const Interp<float, float>& distance_2_z_scale,
    double scale)
{
    for (auto& lst : triangles) {
        for (auto& tri : lst->triangles) {
            for (auto& v : tri.flat_iterable()) {
                auto uv = FixedArray<double, 2>{ v.position(0), v.position(1) };
                uv *= uv_scale / scale;
                uv(0) -= std::floor(uv(0));
                uv(1) -= std::floor(uv(1));
                double displacement;
                if (!bilinear_grayscale_interpolation(uv(1) * double(displacementmap.shape(0) - 1), uv(0) * double(displacementmap.shape(1) - 1), displacementmap, displacement)) {
                    THROW_OR_ABORT("Unexpected bilinear interpolation failure");
                }
                auto dist = ground_bvh.min_dist({v.position(0), v.position(1)}, scale * distance_2_z_scale.xmax());
                v.position += scale * displacement * (v.normal * distance_2_z_scale(float(dist / scale))).casted<double>();
            }
        }
    }
}
