#include "Make_Triangles_With_Opposing_Normals_Two_Sided.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Make_Triangles_With_Opposing_Normals_Two_Sided.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::make_triangles_with_opposing_normals_two_sided(
    const std::string& resource_name,
    SceneNodeResources& scene_node_resources,
    PhysicsMaterial material_filter)
{
    scene_node_resources.add_modifier(
        resource_name,
        [material_filter]
        (ISceneNodeResource& resource)
        {
            auto modify = [material_filter]<class TPos>(
                std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
            {
                cvas.remove_if([&](auto& cva){
                    if (!any(cva->physics_material & material_filter)) {
                        return false;
                    }
                    make_triangles_with_opposing_normals_two_sided(*cva, cvas);
                    return cva->empty();
                    });
            };
            for (const auto& acva : resource.get_rendering_arrays()) {
                modify(acva->scvas);
                modify(acva->dcvas);
            }
        });
}
