#include "Replace_Terrain_Material.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material/Merged_Texture_Filter.hpp>
#include <Mlib/Geometry/Material/Merged_Texture_Name.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Terrain_Uv.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::replace_terrain_material(
    const std::string& resource_name,
    const std::vector<VariableAndHash<std::string>>& textures,
    double scale,
    double uv_scale,
    double uv_period,
    UpAxis up_axis,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources)
{
    scene_node_resources.add_modifier(
        resource_name,
        [scale,
         uv_scale,
         uv_period,
         up_axis,
         textures,
         &rendering_resources]
        (ISceneNodeResource& scene_node_resource){
            auto replace = [&]<typename T>(const std::list<std::shared_ptr<ColoredVertexArray<T>>>& cvas){
                for (const auto& cva : cvas) {
                    if (!cva->modifier_backlog.convert_to_terrain) {
                        continue;
                    }
                    cva->material.textures_color.clear();
                    for (auto& t : textures) {
                        BlendMapTexture bt = rendering_resources.get_blend_map_texture(t);
                        cva->material.textures_color.push_back(bt);
                    }
                    cva->material.blend_mode = BlendMode::OFF;
                    for (auto& t : cva->triangles) {
                        using I = funpack_t<T>;
                        auto uv = terrain_uv(
                            t(0).position,
                            t(1).position,
                            t(2).position,
                            (I)scale,
                            (I)uv_scale,
                            (I)uv_period,
                            up_axis);
                        t(0).uv = uv[0];
                        t(1).uv = uv[1];
                        t(2).uv = uv[2];
                    }
                }
            };
            auto meshes = scene_node_resource.get_rendering_arrays();
            for (const auto& mesh : meshes) {
                replace(mesh->scvas);
                replace(mesh->dcvas);
            }
        });
}
