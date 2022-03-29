#include "Batch_Resource_Instantiator.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

BatchResourceInstantiator::BatchResourceInstantiator()
{}

BatchResourceInstantiator::~BatchResourceInstantiator()
{}

void BatchResourceInstantiator::add_parsed_resource_name(
    const FixedArray<float, 3>& p,
    const ParsedResourceName& prn,
    float yangle,
    float scale)
{
    ResourceInstanceDescriptor rid{
        .position = p,
        .yangle = yangle,
        .scale = scale,
        .billboard_id = prn.billboard_id};
    if (prn.aggregate_mode & (AggregateMode::INSTANCES_ONCE | AggregateMode::INSTANCES_SORTED_CONTINUOUSLY)) {
        resource_instance_positions_[prn.name].push_back(rid);
    } else {
        object_resource_descriptors_.push_back({p, prn.name, scale});
    }
    if (!prn.hitbox.empty()) {
        hitboxes_[prn.hitbox].push_back(rid);
    }
}

void BatchResourceInstantiator::add_parsed_resource_name(
    const FixedArray<float, 2>& p,
    float height,
    const ParsedResourceName& prn,
    float yangle,
    float scale)
{
    add_parsed_resource_name(
        FixedArray<float, 3>{p(0), p(1), height},
        prn,
        yangle,
        scale);
}

void BatchResourceInstantiator::instantiate_renderables(
    const SceneNodeResources& scene_node_resources,
    SceneNode& scene_node,
    const FixedArray<float, 3>& rotation,
    float scale,
    const RenderableResourceFilter& renderable_resource_filter) const
{
    {
        size_t i = 0;
        for (const auto& p : object_resource_descriptors_) {
            auto node = std::make_unique<SceneNode>();
            node->set_position(p.position);
            node->set_scale(scale * p.scale);
            node->set_rotation(rotation);
            scene_node_resources.instantiate_renderable(p.name, p.name, *node, renderable_resource_filter);
            if (node->requires_render_pass()) {
                scene_node.add_child(p.name + "-" + std::to_string(i++), std::move(node));
            } else {
                std::cerr << "Adding aggregate " << p.name << std::endl;
                scene_node.add_aggregate_child(p.name + "-" + std::to_string(i++), std::move(node));
            }
        }
    }
    for (const auto& p : resource_instance_positions_) {
        auto node = std::make_unique<SceneNode>();
        node->set_rotation(rotation);
        scene_node_resources.instantiate_renderable(p.first, p.first, *node, renderable_resource_filter);
        if (node->requires_render_pass()) {
            throw std::runtime_error("Object " + p.first + " requires render pass");
        }
        scene_node.add_instances_child(p.first, std::move(node));
        for (const auto& r : p.second) {
            scene_node.add_instances_position(p.first, r.position, r.yangle, r.billboard_id);
        }
    }
}

void BatchResourceInstantiator::instantiate_hitboxes(
    std::list<std::shared_ptr<ColoredVertexArray>>& cvas,
    const SceneNodeResources& scene_node_resources,
    float scale) const
{
    auto rx = rodrigues2(FixedArray<float, 3>{1.f, 0.f, 0.f}, float{M_PI} / 2.f);
    size_t i = 0;
    for (auto& p : hitboxes_) {
        for (auto& x : scene_node_resources.get_animated_arrays(p.first)->cvas) {
            for (auto& y : p.second) {
                cvas.push_back(
                    x->transformed(
                        TransformationMatrix{
                            scale * dot2d(
                                rodrigues2(FixedArray<float, 3>{0.f, 0.f, 1.f}, y.yangle),
                                rx),
                            y.position},
                    "_transformed_tm_" + std::to_string(i++)));
            }
        }
    }
}

void BatchResourceInstantiator::insert_into(std::list<FixedArray<float, 3>*>& positions) {
    for (auto& d : object_resource_descriptors_) {
        positions.push_back(&d.position);
    }
    for (auto& i : resource_instance_positions_) {
        for (auto& d : i.second) {
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
    std::set<const FixedArray<float, 3>*> vertices_to_delete)
{
    object_resource_descriptors_.remove_if([&vertices_to_delete](const ObjectResourceDescriptor& d){
        return vertices_to_delete.contains(&d.position);
    });
    for (auto& i : resource_instance_positions_) {
        i.second.remove_if([&vertices_to_delete](const ResourceInstanceDescriptor& d){
            return vertices_to_delete.contains(&d.position);
        });
    }
    for (auto& h : hitboxes_) {
        h.second.remove_if([&vertices_to_delete](const ResourceInstanceDescriptor& p){
            return vertices_to_delete.contains(&p.position);
        });
    }
}
