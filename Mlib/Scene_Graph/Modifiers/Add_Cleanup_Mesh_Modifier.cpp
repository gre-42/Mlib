#include "Add_Cleanup_Mesh_Modifier.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Cleanup_Mesh.hpp>
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
    scene_node_resources.add_modifier(
        resource_name,
        [min_distance_material_filter,
        min_vertex_distance,
        modulo_uv]
        (ISceneNodeResource& resource)
        {
            auto cleanup = [&]<class TPos>(
                CleanupMesh<TPos>& ccleanup,
                std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
            {
                cvas.remove_if([&](auto& cva){
                    ccleanup(
                        *cva,
                        min_distance_material_filter,
                        (TPos)min_vertex_distance,
                        modulo_uv);
                    return cva->empty();
                    });
            };
            CleanupMesh<float> scleanup;
            CleanupMesh<CompressedScenePos> dcleanup;
            for (const auto& acva : resource.get_rendering_arrays()) {
                cleanup(scleanup, acva->scvas);
                cleanup(dcleanup, acva->dcvas);
            }
        });
}
