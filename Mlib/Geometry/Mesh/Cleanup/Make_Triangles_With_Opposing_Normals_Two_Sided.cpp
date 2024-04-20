#include "Make_Triangles_With_Opposing_Normals_Two_Sided.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <map>

using namespace Mlib;

template <class TPos>
void Mlib::make_triangles_with_opposing_normals_two_sided(
    ColoredVertexArray<TPos>& cva,
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& result)
{
    if (any(cva.physics_material & PhysicsMaterial::ATTR_TWO_SIDED)) {
        return;
    }
    std::list<FixedArray<ColoredVertex<TPos>, 3>> two_sided_triangles;

    std::map<std::pair<OrderableFixedArray<TPos, 3>, OrderableFixedArray<TPos, 3>>, size_t> edges;
    for (const auto& tri : cva.triangles) {
        for (size_t i = 0; i < 3; ++i) {
            ++edges[{OrderableFixedArray{tri(i).position}, OrderableFixedArray{tri((i + 1) % 3).position}}];
        }
    }
    std::list<FixedArray<ColoredVertex<TPos>, 3>> trimmed;
    for (const auto& tri : cva.triangles) {
        for (size_t i = 0; i < 3; ++i) {
            if (edges.at({OrderableFixedArray{tri(i).position}, OrderableFixedArray{tri((i + 1) % 3).position}}) >= 2) {
                two_sided_triangles.push_back(tri);
                goto skip;
            }
        }
        trimmed.push_back(tri);
        skip:;
    }
    if (!two_sided_triangles.empty()) {
        cva.triangles = std::vector(trimmed.begin(), trimmed.end());
        auto two_sided_array = std::make_shared<ColoredVertexArray<TPos>>(
                cva.name + "_two_sided",
                cva.material,
                cva.physics_material | PhysicsMaterial::ATTR_TWO_SIDED,
            ModifierBacklog{},
            std::vector<FixedArray<ColoredVertex<TPos>, 4>>(),                      // quads
            std::vector(two_sided_triangles.begin(), two_sided_triangles.end()),    // triangles
            std::vector<FixedArray<ColoredVertex<TPos>, 2>>(),                      // lines
            std::vector<FixedArray<std::vector<BoneWeight>, 3>>(),                  // triangle_bone_weights
            std::vector<FixedArray<float, 3>>(),                                    // continous_triangle_texture_layers
            std::vector<FixedArray<uint8_t, 3>>());                                 // discrete_triangle_texture_layers
        two_sided_array->material.cull_faces = false;
        result.push_back(two_sided_array);
    }
}

namespace Mlib {
    template void make_triangles_with_opposing_normals_two_sided<float>(ColoredVertexArray<float>& cva, std::list<std::shared_ptr<ColoredVertexArray<float>>>& result);
    template void make_triangles_with_opposing_normals_two_sided<double>(ColoredVertexArray<double>& cva, std::list<std::shared_ptr<ColoredVertexArray<double>>>& result);
}
