#include "Rendering_Context.hpp"
#include <Mlib/Singleton_Guard.impl.hpp>

using namespace Mlib;

template SingletonGuard<const RenderingContext>::SingletonGuard(const RenderingContext& resource_context);
template SingletonGuard<const RenderingContext>::~SingletonGuard();

template const RenderingContext& Singleton<const RenderingContext>::instance();
template const RenderingContext* Singleton<const RenderingContext>::instance_;

RenderingContextGuard::RenderingContextGuard(const RenderingContext& context)
: SingletonGuard<const RenderingContext>{ context }
{}

RenderingContextGuard RenderingContextGuard::root(
    SceneNodeResources& scene_node_resources,
    ParticleResources& particle_resources,
    RenderingResources& rendering_resources,
    int z_order)
{
    return RenderingContextGuard{RenderingContext{
        .scene_node_resources = scene_node_resources,
        .particle_resources = particle_resources,
        .rendering_resources = rendering_resources,
        .z_order = z_order}};
}

RenderingContextGuard::~RenderingContextGuard() = default;

SceneNodeResources& RenderingContextStack::primary_scene_node_resources() {
    return instance().scene_node_resources;
}

ParticleResources& RenderingContextStack::primary_particle_resources() {
    return instance().particle_resources;
}

RenderingResources& RenderingContextStack::primary_rendering_resources() {
    return instance().rendering_resources;
}

int RenderingContextStack::z_order() {
    return instance().z_order;
}
