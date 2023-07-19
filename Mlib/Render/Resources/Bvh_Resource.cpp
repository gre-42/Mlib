#include "Bvh_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>

using namespace Mlib;

BvhResource::BvhResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas)
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
    const RenderableResourceFilter& renderable_resource_filter,
    const Bvh<float, std::pair<const Material*, const FixedArray<ColoredVertex<float>, 3>*>, 3>& bvh)
{
    if (!bvh.data().empty()) {
        AxisAlignedBoundingBox<float, 3> aabb;
        std::map<const Material*, std::list<const FixedArray<ColoredVertex<float>, 3>*>> cvas;
        for (const auto& b : bvh.data()) {
            cvas[b.second.first].push_back(b.second.second);
            aabb.extend(b.first);
        }
        auto center = (aabb.min() + aabb.max()) / 2.f;
        auto node = std::make_unique<SceneNode>();
        node->set_position((center - position_shift).casted<double>());
        std::list<std::shared_ptr<ColoredVertexArray<float>>> lcvas;
        for (const auto& [material, cva] : cvas) {
            std::vector<FixedArray<ColoredVertex<float>, 3>> vcva(cva.size());
            for (const auto& tri : cva) {
                auto t = *tri;
                for (auto& p : t->flat_iterable()) {
                    p.position -= center;
                }
                vcva.push_back(t);
            }
            lcvas.push_back(std::make_shared<ColoredVertexArray<float>>(
                name,                                                        // name
                *material,                                                   // material
                PhysicsMaterial::ATTR_VISIBLE,                               // physics_material
                std::move(vcva),                                             // triangles
                std::vector<FixedArray<ColoredVertex<float>, 2>>{},          // lines
                std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},       // triangle_bone_weights
                std::vector<FixedArray<std::vector<BoneWeight>, 2>>{},       // line_bone_weights
                std::vector<FixedArray<uint8_t, 3>>{},                       // triangle_texture_layers
                std::vector<FixedArray<uint8_t, 2>>{}));                     // line_texture_layers
            // lcvas.back()->material.is_small = true;
            // lcvas.back()->material.aggregate_mode = AggregateMode::SORTED_CONTINUOUSLY;
        }
        std::make_shared<ColoredVertexArrayResource>(
                lcvas,
                std::list<std::shared_ptr<ColoredVertexArray<double>>>{})->
            instantiate_renderable(InstantiationOptions{
                .instance_name = "renderable_bvh",
                .scene_node = *node,
                .renderable_resource_filter = renderable_resource_filter});
        scene_node.add_child(name + "_data", std::move(node));
    }
    size_t i = 0;
    for (const auto& c : bvh.children()) {
        auto node = std::make_unique<SceneNode>();
        node->set_position(((c.first.min() + c.first.max()) / 2.f - position_shift).casted<double>());
        instantiate_bvh(
            name,
            *node,
            position_shift + node->position().casted<float>(),
            renderable_resource_filter,
            c.second);
        scene_node.add_child("bvh_" + std::to_string(i), std::move(node));
        ++i;
    }
}

void BvhResource::instantiate_renderable(const InstantiationOptions& options) const
{
    instantiate_bvh(options.instance_name, options.scene_node, fixed_zeros<float, 3>(), options.renderable_resource_filter, bvh_);
}
