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
    const TriangleList<double>& triangles,
    float scale,
    float distance)
{
    if (distance == INFINITY) {
        return;
    }
    TriangleSampler2<double> ts{ 1 };
    NormalRandomNumberGenerator<float> scale_rng{ 0, 1.f, 0.2f };
    for (auto& t : triangles.triangles_) {
        ts.sample_triangle_interior<3>(
            t(0).position,
            t(1).position,
            t(2).position,
            distance * scale,
            [&](const double& a, const double& b, const double& c)
            {
                if (auto prn = rnc.try_multiple_times(10); prn != nullptr) {
                    FixedArray<double, 3> p = t(0).position * a + t(1).position * b + t(2).position * c;
                    bri.add_parsed_resource_name(p, *prn, 0.f, scale_rng());
                }
            });
    }
}
