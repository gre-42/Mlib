#include "Renderable_Bvh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

RenderableBvh::RenderableBvh(
    const std::list<std::shared_ptr<ColoredVertexArray>>& cvas)
: cvas_{cvas},
  bvh_{{0.5f, 0.5f, 0.5f}, 10}
{
    for (const auto& cva : cvas) {
        for (const auto& t : cva->triangles) {
            AxisAlignedBoundingBox<float, 3> aabb;
            for (const auto& p : t.flat_iterable()) {
                aabb.extend(p.position);
            }
            bvh_.insert(aabb, {&cva->material, &t});
        }
    }
}

static void instantiate_bvh(
    const std::string& name,
    SceneNode& scene_node,
    const FixedArray<float, 3>& position_shift,
    const SceneNodeResourceFilter& resource_filter,
    const Bvh<float, std::pair<const Material*, const FixedArray<ColoredVertex, 3>*>, 3>& bvh)
{
    if (!bvh.data().empty()) {
        AxisAlignedBoundingBox<float, 3> aabb;
        std::map<const Material*, std::list<const FixedArray<ColoredVertex, 3>*>> cvas;
        for (const auto& b : bvh.data()) {
            cvas[b.second.first].push_back(b.second.second);
            aabb.extend(b.first);
        }
        auto center = (aabb.min() + aabb.max()) / 2.f;
        auto node = new SceneNode();
        node->set_position(center - position_shift);
        std::list<std::shared_ptr<ColoredVertexArray>> lcvas;
        for (const auto& cva : cvas) {
            std::vector<FixedArray<ColoredVertex, 3>> vcva(cva.second.size());
            for (const auto& tri : cva.second) {
                auto t = *tri;
                for (auto& p : t->flat_iterable()) {
                    p.position -= center;
                }
                vcva.push_back(t);
            }
            lcvas.push_back(std::make_shared<ColoredVertexArray>(
                name,                                                    // name
                *cva.first,                                              // material
                std::move(vcva),                                         // triangles
                std::vector<FixedArray<ColoredVertex, 2>>{},             // lines
                std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},   // triangle_bone_weights
                std::vector<FixedArray<std::vector<BoneWeight>, 2>>{})); // line_bone_weights
            lcvas.back()->material.is_small = true;
            lcvas.back()->material.aggregate_mode = AggregateMode::SORTED_CONTINUOUSLY;
        }
        std::make_shared<RenderableColoredVertexArray>(lcvas, nullptr)->
            instantiate_renderable("renderable_bvh", *node, resource_filter);
        scene_node.add_child(name + "_data", node);
    }
    size_t i = 0;
    for (const auto& c : bvh.children()) {
        auto node = new SceneNode();
        node->set_position((c.first.min() + c.first.max()) / 2.f - position_shift);
        instantiate_bvh(
            name,
            *node,
            position_shift + node->position(),
            resource_filter,
            c.second);
        scene_node.add_child("bvh_" + std::to_string(i), node);
        ++i;
    }
}

void RenderableBvh::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    instantiate_bvh(name, scene_node, fixed_zeros<float, 3>(), resource_filter, bvh_);
    std::cerr << scene_node << std::endl;
}
