#include "Modify_Rendering_Material.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <memory>

using namespace Mlib;

void Mlib::modify_rendering_material(
    const VariableAndHash<std::string>& resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& filter,
    std::optional<BlendMode> blend_mode,
    std::optional<ExternalRenderPassType> occluded_pass,
    std::optional<ExternalRenderPassType> occluder_pass,
    std::optional<InterpolationMode> magnifying_interpolation_mode,
    std::optional<std::string> histogram)
{
    scene_node_resources.add_modifier(
        resource_name,
        [filter,
         blend_mode,
         occluded_pass,
         occluder_pass,
         magnifying_interpolation_mode,
         histogram]
        (ISceneNodeResource& resource)
        {
            for (auto& meshes : resource.get_rendering_arrays()) {
                auto patch = [&](auto& cvas) {
                    for (auto& cva : cvas) {
                        if (!filter.matches(*cva)) {
                            continue;
                        }
                        if (blend_mode.has_value()) {
                            cva->material.blend_mode = *blend_mode;
                        }
                        if (occluded_pass.has_value()) {
                            cva->material.occluded_pass = *occluded_pass;
                        }
                        if (occluder_pass.has_value()) {
                            cva->material.occluder_pass = *occluder_pass;
                        }
                        if (magnifying_interpolation_mode.has_value()) {
                            for (auto& texture : cva->material.textures_color) {
                                texture.texture_descriptor.color.magnifying_interpolation_mode = *magnifying_interpolation_mode;
                                texture.texture_descriptor.color.compute_hash();
                            }
                        }
                        if (histogram.has_value()) {
                            for (auto& texture : cva->material.textures_color) {
                                texture.texture_descriptor.color.histogram = *histogram;
                                texture.texture_descriptor.color.hash.reset();
                                texture.texture_descriptor.color.compute_hash();
                            }
                        }
                    }
                };
                patch(meshes->dcvas);
                patch(meshes->scvas);
            }
        });
}
