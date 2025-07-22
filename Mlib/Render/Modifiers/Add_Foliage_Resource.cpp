#include "Add_Foliage_Resource.hpp"
#include <Mlib/Default_Uninitialized_List.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Render/Resources/Foliage_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <memory>

using namespace Mlib;

void Mlib::add_foliage_resource(
    const VariableAndHash<std::string>& mesh_resource_name,
    const VariableAndHash<std::string>& foliage_resource_name,
    SceneNodeResources& scene_node_resources,
    const std::vector<ParsedResourceName>& near_grass_resources,
    const std::vector<ParsedResourceName>& dirty_near_grass_resources,
    ScenePos near_grass_distance,
    const std::string& near_grass_foliagemap,
    float near_grass_foliagemap_scale,
    float scale,
    UpAxis up_axis)
{
    UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>> grass_triangles;
    auto meshes = scene_node_resources.get_rendering_arrays(mesh_resource_name);
    for (const auto& mesh : meshes) {
        for (const auto& cva : mesh->dcvas) {
            if (!cva->modifier_backlog.add_foliage) {
                continue;
            }
            cva->modifier_backlog.add_foliage = false;
            grass_triangles.insert(grass_triangles.end(), cva->triangles.begin(), cva->triangles.end());
        }
    }
    auto res = std::make_shared<FoliageResource>(
        scene_node_resources,
        grass_triangles,
        near_grass_resources,
        dirty_near_grass_resources,
        near_grass_distance,
        near_grass_foliagemap,
        near_grass_foliagemap_scale,
        scale,
        up_axis);
    scene_node_resources.add_resource(
        foliage_resource_name,
        res);
    scene_node_resources.add_companion(
        mesh_resource_name,
        foliage_resource_name,
        RenderableResourceFilter{});
}
