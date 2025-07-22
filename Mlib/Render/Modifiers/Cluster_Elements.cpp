#include "Cluster_Elements.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Instance/Instance_Information.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Cluster_Triangles.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Position_And_Meshes.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <memory>

using namespace Mlib;

template <class TPos, class TWidth>
static void patch(
    SceneNodeResources& scene_node_resources,
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo,
    const FixedArray<TWidth, 3>& width,
    const SquaredStepDistances& center_distances2,
    const GroupAndName& prefix,
    RenderingDynamics rendering_dynamics,
    std::list<VariableAndHash<std::string>>& added_scene_node_resources,
    std::list<VariableAndHash<std::string>>& added_instantiables)
{
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    for (const auto& [i, cs] : enumerate(cluster_triangles(
        cvas,
        cluster_center_by_grid<TPos, TWidth>(width),
        prefix + "_split")))
    {
        auto resource_name = VariableAndHash<std::string>{(prefix + std::to_string(i)).full_name()};
        auto ctrafo = trafo * TransformationMatrix<SceneDir, ScenePos, 3>{
            fixed_identity_array<SceneDir, 3>(),
            cs.position.template casted<ScenePos>()};
        for (const auto& [continuous_blending_z_order, cvas] : cs.cvas) {
            std::list<std::shared_ptr<ColoredVertexArray<float>>> scvas;
            for (const auto& cva : cvas) {
                auto transformed = cva->template translated<float>(-cs.position, "_centered");
                transformed->morphology.center_distances2 = center_distances2;
                scvas.emplace_back(std::move(transformed));
            }
            scene_node_resources.add_resource(
                resource_name,
                std::make_shared<ColoredVertexArrayResource>(std::move(scvas)));
            added_scene_node_resources.push_back(resource_name);
            scene_node_resources.add_instantiable(
                resource_name,
                InstanceInformation<ScenePos>{
                    .resource_name = resource_name,
                    .trafo = ctrafo,
                    .scale = 1,
                    .rendering_dynamics = rendering_dynamics
                });
            added_instantiables.push_back(resource_name);
        }
    }
    cvas = std::move(result);
}

void Mlib::cluster_elements(
    const std::vector<VariableAndHash<std::string>>& resource_names,
    SceneNodeResources& scene_node_resources,
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo,
    const FixedArray<float, 3>& width,
    const SquaredStepDistances& center_distances2,
    RenderingDynamics rendering_dynamics,
    std::list<VariableAndHash<std::string>>& added_scene_node_resources,
    std::list<VariableAndHash<std::string>>& added_instantiables)
{
    for (const auto& resource_name : resource_names) {
        const auto& acvas = scene_node_resources.get_rendering_arrays(resource_name);
        for (auto& acva : acvas) {
            patch<CompressedScenePos, ScenePos>(
                scene_node_resources,
                acva->dcvas,
                trafo,
                width.casted<ScenePos>(),
                center_distances2,
                *resource_name,
                rendering_dynamics,
                added_scene_node_resources,
                added_instantiables);
            patch<float, float>(
                scene_node_resources,
                acva->scvas,
                trafo,
                width,
                center_distances2,
                *resource_name,
                rendering_dynamics,
                added_scene_node_resources,
                added_instantiables);
        }
    }
}
