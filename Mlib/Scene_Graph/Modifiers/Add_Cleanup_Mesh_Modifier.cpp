#include "Add_Cleanup_Mesh_Modifier.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Merge_Neighboring_Points.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Modulo_Uv.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Remove_Degenerate_Triangles.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::add_cleanup_mesh_modifier(
    const std::string& resource_name,
    SceneNodeResources& scene_node_resources,
    float min_vertex_distance,
    PhysicsMaterial min_distance_material_filter,
    bool modulo_uv)
{
    auto min_distance_filter = PhysicsMaterial::NONE;
    scene_node_resources.add_modifier(
        resource_name,
        [min_distance_filter,
        min_vertex_distance,
        modulo_uv]
        (ISceneNodeResource& resource)
        {
            auto cleanup = [min_distance_filter, min_vertex_distance, modulo_uv]<class TPos>(
                Bvh<TPos, FixedArray<TPos, 3>, 3>& bvh,
                std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
            {
                cvas.remove_if([&bvh, min_distance_filter, min_vertex_distance, modulo_uv](auto& cva){
                    if ((min_vertex_distance != 0) &&
                        any(cva->physics_material & min_distance_filter))
                    {
                        merge_neighboring_points<TPos>(*cva, bvh, min_vertex_distance);
                    }
                    remove_degenerate_triangles(*cva);
                    // remove_triangles_with_opposing_normals(*cva);
                    if (modulo_uv) {
                        Mlib::modulo_uv(*cva);
                    }
                    return cva->triangles.empty();
                    });
            };
            Bvh<float, FixedArray<float, 3>, 3> sbvh{FixedArray<float, 3>{0.1f, 0.1f, 0.1f}, 17};
            Bvh<double, FixedArray<double, 3>, 3> dbvh{FixedArray<double, 3>{0.1, 0.1, 0.1}, 17};
            for (const auto& acva : resource.get_rendering_arrays()) {
                cleanup(sbvh, acva->scvas);
                cleanup(dbvh, acva->dcvas);
            }
        });
}
