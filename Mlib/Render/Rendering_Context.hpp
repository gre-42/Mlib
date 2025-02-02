#pragma once
#include <Mlib/Singleton_Guard.hpp>

namespace Mlib {

struct RenderConfig;
struct RenderingContext;
class SceneNodeResources;
class ParticleResources;
class TrailResources;
class RenderingResources;

struct RenderingContext {
    SceneNodeResources& scene_node_resources;
    ParticleResources& particle_resources;
    TrailResources& trail_resources;
    RenderingResources& rendering_resources;
    int z_order;
};

class RenderingContextGuard: public SingletonGuard<RenderingContext> {
    RenderingContextGuard(const RenderingContextGuard&) = delete;
    RenderingContextGuard& operator = (const RenderingContextGuard&) = delete;
public:
    explicit RenderingContextGuard(RenderingContext& context);
    ~RenderingContextGuard();
};

class RenderingContextStack: public Singleton<RenderingContext> {
    friend RenderingContextGuard;
public:
    static SceneNodeResources& primary_scene_node_resources();
    static ParticleResources& primary_particle_resources();
    static TrailResources& primary_trail_resources();
    static RenderingResources& primary_rendering_resources();
    static int z_order();
};

}
