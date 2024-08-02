#include "Rendering_Context.hpp"
#include <Mlib/Singleton_Guard.impl.hpp>

using namespace Mlib;

template SingletonGuard<const RenderingContext>::SingletonGuard(const RenderingContext& resource_context);
template SingletonGuard<const RenderingContext>::~SingletonGuard();

template const RenderingContext& Singleton<const RenderingContext>::instance();
template const RenderingContext* Singleton<const RenderingContext>::instance_;
template SafeAtomicSharedMutex Singleton<const RenderingContext>::mutex_;

RenderingContextGuard::RenderingContextGuard(RenderingContext& context)
: SingletonGuard<RenderingContext>{ context }
{}

RenderingContextGuard::~RenderingContextGuard() = default;

SceneNodeResources& RenderingContextStack::primary_scene_node_resources() {
    return instance().scene_node_resources;
}

ParticleResources& RenderingContextStack::primary_particle_resources() {
    return instance().particle_resources;
}

TrailResources& RenderingContextStack::primary_trail_resources() {
    return instance().trail_resources;
}

RenderingResources& RenderingContextStack::primary_rendering_resources() {
    return instance().rendering_resources;
}

int RenderingContextStack::z_order() {
    return instance().z_order;
}
