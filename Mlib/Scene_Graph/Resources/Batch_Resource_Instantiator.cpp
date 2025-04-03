#include "Batch_Resource_Instantiator.hpp"
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Descriptors/Object_Resource_Descriptor.hpp>
#include <Mlib/Scene_Graph/Descriptors/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IImposters.hpp>
#include <Mlib/Scene_Graph/Interfaces/ISupply_Depots.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

BatchResourceInstantiator::BatchResourceInstantiator(
    const FixedArray<float, 3>& rotation,
    float scale,
    HitboxContainer hitbox_container)
    : rotation_{ rotation }
    , scale_{ scale }
    , hitbox_container_{ hitbox_container }
{}

BatchResourceInstantiator::~BatchResourceInstantiator()
{}

void BatchResourceInstantiator::add_parsed_resource_name(
    const FixedArray<CompressedScenePos, 3>& p,
    const ParsedResourceName& prn,
    float dyangle,
    float scale)
{
    float yangle = prn.yangle + dyangle;
    if (any(prn.aggregate_mode & (AggregateMode::INSTANCES_ONCE | AggregateMode::INSTANCES_SORTED_CONTINUOUSLY))) {
        ResourceInstanceDescriptor rid{
            .position = p,
            .yangle = yangle,
            .scale = scale,
            .billboard_id = prn.billboard_id};
        resource_instance_positions_[prn.name].push_back(rid);
    } else {
        object_resource_descriptors_.push_back(ObjectResourceDescriptor{
            .position = p,
            .yangle = yangle,
            .name = prn.name,
            .scale = scale,
            .aggregate_mode = prn.aggregate_mode,
            .create_imposter = prn.create_imposter,
            .max_imposter_texture_size = prn.max_imposter_texture_size,
            .supplies = prn.supplies,
            .supplies_cooldown = prn.supplies_cooldown});
    }
    if (!prn.hitbox->empty()) {
        ResourceInstanceDescriptor rid{
            .position = p,
            .yangle = yangle,
            .scale = scale,
            .billboard_id = BILLBOARD_ID_NONE};
        add_hitbox(prn.hitbox, rid);
    }
}

void BatchResourceInstantiator::add_parsed_resource_name(
    const FixedArray<CompressedScenePos, 2>& p,
    CompressedScenePos height,
    const ParsedResourceName& prn,
    float yangle,
    float scale)
{
    add_parsed_resource_name(
        FixedArray<CompressedScenePos, 3>{p(0), p(1), height},
        prn,
        yangle,
        scale);
}

void BatchResourceInstantiator::add_hitbox(
    const VariableAndHash<std::string>& name,
    const ResourceInstanceDescriptor& rid)
{
    if (hitbox_container_ == HitboxContainer::TEMPORARY) {
        hitboxes_[name].push_back(rid);
    } else if (hitbox_container_ == HitboxContainer::INSTANCES) {
        resource_instance_positions_[name].push_back(rid);
    } else {
        THROW_OR_ABORT("Unknown hitbox container");
    }
}

void BatchResourceInstantiator::preload(
    const SceneNodeResources& scene_node_resources,
    const RenderableResourceFilter& filter) const {
    for (const auto& p : object_resource_descriptors_) {
        scene_node_resources.preload_single(*p.name, filter);
    }
    for (const auto& [name, _] : resource_instance_positions_) {
        scene_node_resources.preload_single(*name, filter);
    }
}

void BatchResourceInstantiator::instantiate_root_renderables(
    const SceneNodeResources& scene_node_resources,
    const RootInstantiationOptions& options) const
{
    {
        auto lr = tait_bryan_angles_2_matrix(rotation_);
        for (const auto&& [i, p] : enumerate(object_resource_descriptors_)) {
            if (!p.supplies.empty() && (options.supply_depots == nullptr)) {
                THROW_OR_ABORT("Supplies requested, but no supply depots available");
            }

            auto cm =
                options.absolute_model_matrix *
                TransformationMatrix<float, ScenePos, 3>{
                    dot2d(lr, rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, p.yangle)),
                    p.position.casted<ScenePos>()};
            auto node = make_unique_scene_node(
                cm.t,
                matrix_2_tait_bryan_angles(cm.R),
                p.scale,
                PoseInterpolationMode::DISABLED);

            scene_node_resources.instantiate_child_renderable(
                *p.name,
                ChildInstantiationOptions{
                    .rendering_resources = options.rendering_resources,
                    .instance_name = p.name,
                    .scene_node = node.ref(DP_LOC),
                    .interpolation_mode = PoseInterpolationMode::DISABLED,
                    .renderable_resource_filter = options.renderable_resource_filter });
            std::string node_name = *p.name + "-" + std::to_string(i);
            if (!p.supplies.empty()) {
                options.supply_depots->add_supply_depot(node.ref(DP_LOC), p.supplies, p.supplies_cooldown);
                options.scene.auto_add_root_node(node_name, std::move(node), RenderingDynamics::MOVING);
            } else {
                if (p.aggregate_mode == AggregateMode::NONE) {
                    if (p.create_imposter) {
                        if (options.imposters == nullptr) {
                            THROW_OR_ABORT("Imposter requested, but no imposters available");
                        }
                        options.imposters->create_imposter(node.ref(DP_LOC), node_name, p.max_imposter_texture_size);
                    }
                    options.scene.auto_add_root_node(
                        node_name,
                        std::move(node),
                        RenderingDynamics::STATIC);
                } else {
                    if (any(p.aggregate_mode & ~AggregateMode::OBJECT_MASK)) {
                        THROW_OR_ABORT("Unexpected aggregate mode");
                    }
                    if (p.create_imposter) {
                        THROW_OR_ABORT("Cannot create imposter for aggregate node");
                    }
                    lerr() << "Adding aggregate " << *p.name;
                    options.scene.auto_add_root_node(
                        node_name,
                        std::move(node),
                        RenderingDynamics::STATIC);
                }
            }
        }
    }
    if (!resource_instance_positions_.empty()) {
        auto world_node = make_unique_scene_node(
            options.absolute_model_matrix.t,
            matrix_2_tait_bryan_angles(options.absolute_model_matrix.R),
            1.f,
            PoseInterpolationMode::DISABLED);
        auto scale = options.absolute_model_matrix.get_scale();

        for (const auto& [name, ps] : resource_instance_positions_) {
            auto node = make_unique_scene_node(
                fixed_zeros<ScenePos, 3>(),
                rotation_,
                scale,
                PoseInterpolationMode::DISABLED);
            scene_node_resources.instantiate_child_renderable(
                *name,
                ChildInstantiationOptions{
                    .rendering_resources = options.rendering_resources,
                    .instance_name = name,
                    .scene_node = node.ref(DP_LOC),
                    .interpolation_mode = PoseInterpolationMode::DISABLED,
                    .renderable_resource_filter = options.renderable_resource_filter});
            if (node->requires_render_pass(ExternalRenderPassType::STANDARD)) {
                THROW_OR_ABORT("Object " + *name + " requires render pass");
            }
            world_node->add_instances_child(*name, std::move(node));
            for (const auto& r : ps) {
                world_node->add_instances_position(*name, r.position, r.yangle, r.billboard_id);
            }
        }
        options.scene.auto_add_root_node(
            *options.instance_name + "_inst_world",
            std::move(world_node),
            RenderingDynamics::STATIC);
    }
    // if (!resource_instance_positions_.empty()) {
    //     options.scene_node.optimize_instances_search_time(lraw());
    // }
}

void BatchResourceInstantiator::instantiate_hitboxes(
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    const SceneNodeResources& scene_node_resources) const
{
    auto rx = tait_bryan_angles_2_matrix(rotation_);
    size_t i = 0;
    for (auto& [name, ps] : hitboxes_)
    {
        auto add_hitbox = [&, &name=name, &ps=ps]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& local_cvas){
            for (auto& x : local_cvas) {
                for (auto& y : ps) {
                    cvas.push_back(
                        x->template transformed<CompressedScenePos>(
                            TransformationMatrix{
                                scale_ * dot2d(
                                    rodrigues2(FixedArray<float, 3>{0.f, 0.f, 1.f}, y.yangle),
                                    rx),
                                funpack(y.position)},
                        '_' + *name + "_transformed_tm_" + std::to_string(i++)));
                }
            }
        };
        auto acva = scene_node_resources.get_physics_arrays(*name);
        add_hitbox(acva->scvas);
        if constexpr (std::is_same_v<ScenePos, double>) {
            add_hitbox(acva->dcvas);
        } else {
            if (!acva->dcvas.empty()) {
                THROW_OR_ABORT("Scene position is single precision, but double arrays exist");
            }
        }
    }
}

void BatchResourceInstantiator::insert_into(std::list<FixedArray<CompressedScenePos, 3>*>& positions) {
    for (auto& d : object_resource_descriptors_) {
        positions.push_back(&d.position);
    }
    for (auto& [name, ps] : resource_instance_positions_) {
        for (auto& d : ps) {
            positions.push_back(&d.position);
        }
    }
    for (auto& h : hitboxes_) {
        for (auto& d : h.second) {
            positions.push_back(&d.position);
        }
    }
}

void BatchResourceInstantiator::remove(
    std::set<const FixedArray<CompressedScenePos, 3>*> vertices_to_delete)
{
    object_resource_descriptors_.remove_if([&vertices_to_delete](const ObjectResourceDescriptor& d){
        return vertices_to_delete.contains(&d.position);
    });
    for (auto& [_, ps] : resource_instance_positions_) {
        ps.remove_if([&vertices_to_delete](const ResourceInstanceDescriptor& d){
            return vertices_to_delete.contains(&d.position);
        });
    }
    for (auto& [_, hs] : hitboxes_) {
        hs.remove_if([&vertices_to_delete](const ResourceInstanceDescriptor& p){
            return vertices_to_delete.contains(&p.position);
        });
    }
}

std::list<FixedArray<CompressedScenePos, 3>> BatchResourceInstantiator::hitbox_positions() const {
    std::list<FixedArray<CompressedScenePos, 3>> result;
    for (const auto& [_, hs] : hitboxes_) {
        for (const auto& h : hs) {
            result.push_back(h.position);
        }
    }
    return result;
}
