#pragma once
#include <Mlib/Singleton_Guard.hpp>

namespace Mlib {

struct RenderConfig;
struct RenderingContext;
class SceneNodeResources;
class ParticleResources;
class RenderingResources;

struct RenderingContext {
    SceneNodeResources& scene_node_resources;
    ParticleResources& particle_resources;
    RenderingResources& rendering_resources;
    int z_order;
};

class RenderingContextGuard: public SingletonGuard<const RenderingContext> {
    RenderingContextGuard(const RenderingContextGuard&) = delete;
    RenderingContextGuard& operator = (const RenderingContextGuard&) = delete;
public:
    explicit RenderingContextGuard(const RenderingContext& context);
    ~RenderingContextGuard();
    static RenderingContextGuard root(
        SceneNodeResources& scene_node_resources,
        ParticleResources& particle_resources,
        RenderingResources& rendering_resources,
        int z_order);
};

class RenderingContextStack: public Singleton<const RenderingContext> {
    friend RenderingContextGuard;
public:
    static SceneNodeResources& primary_scene_node_resources();
    static ParticleResources& primary_particle_resources();
    static RenderingResources& primary_rendering_resources();
    static int z_order();
};

}
