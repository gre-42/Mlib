#include "Batch_Resource_Instantiator.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Descriptors/Object_Resource_Descriptor.hpp>
#include <Mlib/Scene_Graph/Descriptors/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IImpostors.hpp>
#include <Mlib/Scene_Graph/Interfaces/ISupply_Depots.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

BatchResourceInstantiator::BatchResourceInstantiator()
{}

BatchResourceInstantiator::~BatchResourceInstantiator()
{}

void BatchResourceInstantiator::add_parsed_resource_name(
    const FixedArray<double, 3>& p,
    const ParsedResourceName& prn,
    float yangle,
    float scale)
{
    ResourceInstanceDescriptor rid{
        .position = p,
        .yangle = yangle,
        .scale = scale,
        .billboard_id = prn.billboard_id};
    if (any(prn.aggregate_mode & (AggregateMode::INSTANCES_ONCE | AggregateMode::INSTANCES_SORTED_CONTINUOUSLY))) {
        resource_instance_positions_[prn.name].push_back(rid);
    } else {
        object_resource_descriptors_.push_back(ObjectResourceDescriptor{
            .position = p,
            .yangle = yangle,
            .name = prn.name,
            .scale = scale,
            .aggregate_mode = prn.aggregate_mode,
            .create_impostor = prn.create_impostor,
            .supplies = prn.supplies,
            .supplies_cooldown = prn.supplies_cooldown});
    }
    if (!prn.hitbox.empty()) {
        hitboxes_[prn.hitbox].push_back(rid);
    }
}

void BatchResourceInstantiator::add_parsed_resource_name(
    const FixedArray<double, 2>& p,
    float height,
    const ParsedResourceName& prn,
    float yangle,
    float scale)
{
    add_parsed_resource_name(
        FixedArray<double, 3>{p(0), p(1), height},
        prn,
        yangle,
        scale);
}

void BatchResourceInstantiator::preload(const SceneNodeResources& scene_node_resources) const {
    for (const auto& p : object_resource_descriptors_) {
        scene_node_resources.preload_single(p.name);
    }
    for (const auto& [name, _] : resource_instance_positions_) {
        scene_node_resources.preload_single(name);
    }
}

void BatchResourceInstantiator::instantiate_renderables(
    const SceneNodeResources& scene_node_resources,
    const InstantiationOptions& options,
    const FixedArray<float, 3>& rotation,
    float scale) const
{
    {
        size_t i = 0;
        for (const auto& p : object_resource_descriptors_) {
            auto unode = std::make_unique<SceneNode>();
            SceneNode* node = unode.get();
            std::string child_name = p.name + "-" + std::to_string(i++);
            auto local_rotation = dot2d(
                tait_bryan_angles_2_matrix(rotation),
                rodrigues2(FixedArray<float, 3>{0.f, 1.0, 0.f}, p.yangle));
            if (!p.supplies.empty()) {
                if (options.supply_depots == nullptr) {
                    throw std::runtime_error("Supplies requested, but no supply depots available");
                }
                auto pm = options.scene_node.absolute_model_matrix();
                auto cm = pm * TransformationMatrix<float, double, 3>{local_rotation, p.position};
                node->set_relative_pose(cm.t(), matrix_2_tait_bryan_angles(cm.R()), p.scale);
                options.scene_node.scene().add_root_node(child_name, std::move(unode));
                options.supply_depots->add_supply_depot(*node, p.supplies, p.supplies_cooldown);
            } else {
                node->set_position(p.position);
                node->set_scale(scale * p.scale);
                node->set_rotation(matrix_2_tait_bryan_angles(local_rotation));
                if (p.aggregate_mode == AggregateMode::NONE) {
                    options.scene_node.add_child(child_name, std::move(unode));
                    if (p.create_impostor) {
                        if (options.impostors == nullptr) {
                            throw std::runtime_error("Impostor requested, but no impostors available");
                        }
                        options.impostors->create_impostor(*node);
                    }
                } else {
                    if ((p.aggregate_mode | AggregateMode::OBJECT_MASK) != AggregateMode::OBJECT_MASK) {
                        throw std::runtime_error("Unexpected aggregate mode");
                    }
                    if (p.create_impostor) {
                        throw std::runtime_error("Cannot create impostor for aggregate node");
                    }
                    std::cerr << "Adding aggregate " << p.name << std::endl;
                    options.scene_node.add_aggregate_child(child_name, std::move(unode));
                }
            }
            scene_node_resources.instantiate_renderable(
                p.name,
                InstantiationOptions{
                    .supply_depots = options.supply_depots,
                    .instance_name = p.name,
                    .scene_node = *node,
                    .renderable_resource_filter = options.renderable_resource_filter});
        }
    }
    for (const auto& [name, ps] : resource_instance_positions_) {
        auto node = std::make_unique<SceneNode>();
        node->set_rotation(rotation);
        scene_node_resources.instantiate_renderable(
            name,
            InstantiationOptions{
                .supply_depots = options.supply_depots,
                .instance_name = name,
                .scene_node = *node,
                .renderable_resource_filter = options.renderable_resource_filter});
        if (node->requires_render_pass(ExternalRenderPassType::STANDARD)) {
            throw std::runtime_error("Object " + name + " requires render pass");
        }
        options.scene_node.add_instances_child(name, std::move(node));
        for (const auto& r : ps) {
            options.scene_node.add_instances_position(name, r.position, r.yangle, r.billboard_id);
        }
    }
}

void BatchResourceInstantiator::instantiate_hitboxes(
    std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas,
    const SceneNodeResources& scene_node_resources,
    float scale) const
{
    auto rx = rodrigues2(FixedArray<float, 3>{1.f, 0.f, 0.f}, 90.f * degrees);
    size_t i = 0;
    for (auto& [name, ps] : hitboxes_)
    {
        auto add_hitbox = [&, &ps=ps]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& local_cvas){
            for (auto& x : local_cvas) {
                for (auto& y : ps) {
                    cvas.push_back(
                        x TEMPLATE transformed<double>(
                            TransformationMatrix{
                                scale * dot2d(
                                    rodrigues2(FixedArray<float, 3>{0.f, 0.f, 1.f}, y.yangle),
                                    rx),
                                y.position},
                        "_transformed_tm_" + std::to_string(i++)));
                }
            }
        };
        auto acva = scene_node_resources.get_animated_arrays(name);
        add_hitbox(acva->scvas);
        add_hitbox(acva->dcvas);
    }
}

void BatchResourceInstantiator::insert_into(std::list<FixedArray<double, 3>*>& positions) {
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
    std::set<const FixedArray<double, 3>*> vertices_to_delete)
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

std::list<FixedArray<double, 3>> BatchResourceInstantiator::hitbox_positions() const {
    std::list<FixedArray<double, 3>> result;
    for (const auto& [_, hs] : hitboxes_) {
        for (const auto& h : hs) {
            result.push_back(h.position);
        }
    }
    return result;
}
