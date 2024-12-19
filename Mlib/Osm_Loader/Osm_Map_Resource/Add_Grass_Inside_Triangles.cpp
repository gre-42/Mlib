#include "Add_Grass_Inside_Triangles.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>

using namespace Mlib;

void Mlib::add_grass_inside_triangles(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const TriangleList<CompressedScenePos>& triangles,
    float scale,
    CompressedScenePos distance)
{
    TriangleSampler2<CompressedScenePos> ts{ 1 };
    FastNormalRandomNumberGenerator<float> scale_rng{ 0, 1.f, 0.2f };
    for (auto& t : triangles.triangles) {
        FixedArray<CompressedScenePos, 3, 3> triangle{
            t(0).position,
            t(1).position,
            t(2).position };
        ts.sample_triangle_interior<3>(
            triangle,
            distance * scale,
            [&](const FixedArray<double, 3>& bcs)
            {
                if (auto prn = rnc.try_multiple_times(10); prn != nullptr) {
                    FixedArray<double, 3> p = dot(bcs, funpack(triangle));
                    bri.add_parsed_resource_name(p.casted<CompressedScenePos>(), *prn, 0.f, scale_rng());
                }
            });
    }
}
