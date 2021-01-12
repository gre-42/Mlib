#include "Renderable_Bvh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

RenderableBvh::RenderableBvh(
    const std::list<std::shared_ptr<ColoredVertexArray>>& cvas,
    RenderingResources& rendering_resources)
: cvas_{cvas},
  bvh_{{0.1, 0.1, 0.1}, 10},
  rendering_resources_{rendering_resources}
{
    for (const auto& cva : cvas) {
        for (const auto& t : cva->triangles) {
            AxisAlignedBoundingBox<float, 3> aabb;
            aabb.extend(t(0).position);
            aabb.extend(t(1).position);
            aabb.extend(t(2).position);
            bvh_.insert(aabb, {&cva->material, &t});
        }
    }
}

static void instantiate_bvh(
    const std::string& name,
    SceneNode& scene_node,
    const SceneNodeResourceFilter& resource_filter,
    const Bvh<float, std::pair<const Material*, const FixedArray<ColoredVertex, 3>*>, 3>& bvh,
    RenderingResources& rendering_resources)
{
    if (!bvh.data().empty()) {
        std::map<const Material*, std::list<const FixedArray<ColoredVertex, 3>*>> cvas;
        for (const auto& b : bvh.data()) {
            cvas[b.second.first].push_back(b.second.second);
        }
        std::list<std::shared_ptr<ColoredVertexArray>> lcvas;
        for (const auto& cva : cvas) {
            std::vector<FixedArray<ColoredVertex, 3>> vcva(cva.second.size());
            for (const auto& tri : cva.second) {
                vcva.push_back(*tri);
            }
            lcvas.push_back(std::make_shared<ColoredVertexArray>(
                name,                                                    // name
                *cva.first,                                              // material
                std::move(vcva),                                         // triangles
                std::vector<FixedArray<ColoredVertex, 2>>{},             // lines
                std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},   // triangle_bone_weights
                std::vector<FixedArray<std::vector<BoneWeight>, 2>>{})); // line_bone_weights
            lcvas.back()->material.is_small = true;
        }
        std::make_shared<RenderableColoredVertexArray>(lcvas, nullptr, rendering_resources)->
            instantiate_renderable("renderable_bvh", scene_node, resource_filter);
    }
    size_t i = 0;
    for (const auto& c : bvh.children()) {
        auto node = new SceneNode();
        instantiate_bvh(name, *node, resource_filter, c.second, rendering_resources);
        scene_node.add_child(
            "bvh_" + std::to_string(i),
            node,
            false);  // false = is_registered
        ++i;
    }
}

void RenderableBvh::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    instantiate_bvh(name, scene_node, resource_filter, bvh_, rendering_resources_);
}
