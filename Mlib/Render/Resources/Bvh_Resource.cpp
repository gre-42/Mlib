#include "Bvh_Resource.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Pos.hpp>

using namespace Mlib;

BvhResource::BvhResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas)
    : cvas_{ cvas }
    , bvh_{ {0.5f, 0.5f, 0.5f}, 10 }
{
    for (const auto& cva : cvas) {
        for (const auto& t : cva->triangles) {
            auto aabb = AxisAlignedBoundingBox<float, 3>::empty();
            for (const auto& p : t.flat_iterable()) {
                aabb.extend(p.position);
            }
            bvh_.insert(aabb, { cva, t });
        }
    }
}

static void instantiate_bvh(
    const VariableAndHash<std::string>& name,
    const DanglingRef<SceneNode>& scene_node,
    const FixedArray<float, 3>& position_shift,
    const RenderableResourceFilter& renderable_resource_filter,
    const Bvh<float, BvhResourcePayload, 3>& bvh,
    const ChildInstantiationOptions& options)
{
    if (!bvh.data().empty()) {
        auto aabb = AxisAlignedBoundingBox<float, 3>::empty();
        std::map<std::shared_ptr<ColoredVertexArray<float>>, std::list<const FixedArray<ColoredVertex<float>, 3>*>> cvas;
        for (const auto& [db, dv] : bvh.data()) {
            cvas[dv.cva].push_back(&dv.triangle);
            aabb.extend(db);
        }
        auto center = (aabb.min() + aabb.max()) / 2.f;
        auto node = make_dunique<SceneNode>(
            (center - position_shift).casted<ScenePos>(),
            fixed_zeros<float, 3>(),
            1.f);
        std::list<std::shared_ptr<ColoredVertexArray<float>>> lcvas;
        for (const auto& [mm, cva] : cvas) {
            UUVector<FixedArray<ColoredVertex<float>, 3>> vcva(cva.size());
            for (const auto& tri : cva) {
                auto t = *tri;
                for (auto& p : t->flat_iterable()) {
                    p.position -= center;
                }
                vcva.push_back(t);
            }
            lcvas.push_back(std::make_shared<ColoredVertexArray<float>>(
                *name,                                                      // name
                mm->material,                                               // material
                mm->morphology,                                             // physics_material
                ModifierBacklog{},                                          // modifier_backlog
                UUVector<FixedArray<ColoredVertex<float>, 4>>{},            // quads
                std::move(vcva),                                            // triangles
                UUVector<FixedArray<ColoredVertex<float>, 2>>{},            // lines
                UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},         // triangle_bone_weights
                UUVector<FixedArray<float, 3>>{},                           // continuous_triangle_texture_layers
                UUVector<FixedArray<uint8_t, 3>>{}));                       // discrete_triangle_texture_layers
            // lcvas.back()->material.is_small = true;
            // lcvas.back()->material.aggregate_mode = AggregateMode::SORTED_CONTINUOUSLY;
        }
        std::make_shared<ColoredVertexArrayResource>(
                lcvas,
                std::list<std::shared_ptr<ColoredVertexArray<double>>>{})->
            instantiate_child_renderable(ChildInstantiationOptions{
                .instance_name = VariableAndHash<std::string>{ "renderable_bvh" },
                .scene_node = node.ref(DP_LOC),
                .interpolation_mode = options.interpolation_mode,
                .renderable_resource_filter = renderable_resource_filter});
        scene_node->add_child(*name + "_data", std::move(node));
    }
    size_t i = 0;
    for (const auto& [cb, cv] : bvh.children()) {
        auto node = make_dunique<SceneNode>(
            ((cb.min() + cb.max()) / 2.f - position_shift).casted<ScenePos>(),
            fixed_zeros<float, 3>(),
            1.f);
        instantiate_bvh(
            name,
            node.ref(DP_LOC),
            position_shift + node->position().casted<float>(),
            renderable_resource_filter,
            cv,
            options);
        scene_node->add_child("bvh_" + std::to_string(i), std::move(node));
        ++i;
    }
}

void BvhResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    instantiate_bvh(
        options.instance_name,
        options.scene_node,
        fixed_zeros<float, 3>(),
        options.renderable_resource_filter,
        bvh_,
        options);
}
