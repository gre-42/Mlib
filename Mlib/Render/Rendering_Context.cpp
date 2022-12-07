#include "Rendering_Context.hpp"
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Resource_Context.impl.hpp>
#include <ostream>

using namespace Mlib;

// thread_local std::list<RenderingContext> RenderingContextStack::context_stack_;

RenderingContextGuard::RenderingContextGuard(const RenderingContext& context)
: ResourceContextGuard<const RenderingContext>{ context }
{}

RenderingContextGuard RenderingContextGuard::root(
    SceneNodeResources& scene_node_resources,
    const std::string& name,
    unsigned int max_anisotropic_filtering_level,
    int z_order)
{
    if (RenderingContextStack::primary_resource_context_ != nullptr) {
        throw std::runtime_error("RenderingContextGuard::root on non-empty stack");
    }
    return RenderingContextGuard{RenderingContext{
        .scene_node_resources = scene_node_resources,
        .rendering_resources = std::make_shared<RenderingResources>(name, max_anisotropic_filtering_level),
        .z_order = z_order}};
}

RenderingContextGuard RenderingContextGuard::layer(
    SceneNodeResources& scene_node_resources,
    const std::string& name,
    unsigned int max_anisotropic_filtering_level,
    int z_order)
{
    if (RenderingContextStack::primary_resource_context_ == nullptr) {
        throw std::runtime_error("RenderingContextGuard::layer on empty stack");
    }
    return RenderingContextGuard{RenderingContext{
        .scene_node_resources = scene_node_resources,
        .rendering_resources = std::make_shared<RenderingResources>(name, max_anisotropic_filtering_level),
        .z_order = z_order}};
}

RenderingContextGuard::~RenderingContextGuard() = default;

SceneNodeResources& RenderingContextStack::primary_scene_node_resources() {
    return primary_resource_context().scene_node_resources;
}

std::shared_ptr<RenderingResources> RenderingContextStack::primary_rendering_resources() {
    return primary_resource_context().rendering_resources;
}

std::shared_ptr<RenderingResources> RenderingContextStack::rendering_resources() {
    return resource_context().rendering_resources;
}

int RenderingContextStack::z_order() {
    return resource_context().z_order;
}

template ResourceContextGuard<const RenderingContext>::ResourceContextGuard(const RenderingContext& resource_context);
template ResourceContextGuard<const RenderingContext>::~ResourceContextGuard();

template const RenderingContext& ResourceContextStack<const RenderingContext>::primary_resource_context();
template const RenderingContext& ResourceContextStack<const RenderingContext>::resource_context();
template const RenderingContext* ResourceContextStack<const RenderingContext>::primary_resource_context_;
template const RenderingContext* ResourceContextStack<const RenderingContext>::secondary_resource_context_;
template std::function<std::function<void()>(std::function<void()>)>
    ResourceContextStack<const RenderingContext>::generate_thread_runner(
        const RenderingContext& primary_resource_context,
        const RenderingContext& secondary_resource_context);
