#include "Add_Shade_Flat_Modifier.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Shade_Flat.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::add_shade_flat_modifier(
    const std::string& resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& filter)
{
    scene_node_resources.add_modifier(
        resource_name,
        [filter](ISceneNodeResource& resource)
        {
            for (const auto& acva : resource.get_rendering_arrays()) {
                for (const auto& cva : acva->scvas) {
                    if (filter.matches(*cva)) {
                        shade_flat(*cva);
                    }
                }
                for (const auto& cva : acva->dcvas) {
                    if (filter.matches(*cva)) {
                        shade_flat(*cva);
                    }
                }
            }
        });
}
