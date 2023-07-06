#include "Modify_Texture_Interpolation_Mode.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <memory>

using namespace Mlib;

void Mlib::modify_texture_interpolation_mode(
    const std::string& resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& filter,
    InterpolationMode magnifying_interpolation_mode)
{
    scene_node_resources.add_modifier(
        resource_name,
        [&filter,
         magnifying_interpolation_mode]
        (ISceneNodeResource& resource)
        {
            for (auto& meshes : resource.get_rendering_arrays()) {
                auto patch = [&filter, magnifying_interpolation_mode](auto& cvas) {
                    for (auto& cva : cvas) {
                        if (!filter.matches(*cva)) {
                            continue;
                        }
                        cva->material.magnifying_interpolation_mode = magnifying_interpolation_mode;
                    }
                };
                patch(meshes->dcvas);
                patch(meshes->scvas);
            }
        });
}
