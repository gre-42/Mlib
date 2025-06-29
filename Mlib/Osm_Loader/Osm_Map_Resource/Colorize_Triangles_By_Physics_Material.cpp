#include "Colorize_Triangles_By_Physics_Material.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

void Mlib::colorize_triangles_by_physics_material(
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas)
{
    for (auto& cva : cvas) {
        for (auto& t : cva->triangles) {
            for (auto& v : t.flat_iterable()) {
                if (any(cva->morphology.physics_material & PhysicsMaterial::OBJ_GROUND)) {
                    v.color = Colors::from_rgb({1.f, 0.f, 0.f});
                }
                if (any(cva->morphology.physics_material & PhysicsMaterial::OBJ_WAY_AIR_SUPPORT)) {
                    v.color = Colors::from_rgb({0.f, 1.f, 0.f});
                }
            }
        }
    }
}
