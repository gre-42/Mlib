#include "Add_Grass_Inside_Triangles.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>

using namespace Mlib;

void Mlib::add_grass_inside_triangles(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    ResourceNameCycle& rnc,
    const TriangleList& triangles,
    float scale,
    float distance)
{
    if (distance == INFINITY) {
        return;
    }
    TriangleSampler2<float> ts{ 1 };
    NormalRandomNumberGenerator<float> scale_rng{ 0, 1.f, 0.2f };
    for (auto& t : triangles.triangles_) {
        ts.sample_triangle_interior<3>(
            t(0).position,
            t(1).position,
            t(2).position,
            distance * scale,
            [&](float a, float b, float c)
            {
                FixedArray<float, 3> p = t(0).position * a + t(1).position * b + t(2).position * c;
                add_parsed_resource_name(p, rnc(), 0.f, scale_rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
            });
    }
}
