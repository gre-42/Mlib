#pragma once
#include <Mlib/Misc/Singleton_Guard.hpp>

namespace Mlib {

struct RenderConfig;
struct RenderingContext;
class SceneNodeResources;
class ParticleResources;
class TrailResources;
class RenderingResources;
class IGpuObjectFactory;
class IGpuVertexArrayRenderer;

struct RenderingContext {
    SceneNodeResources& scene_node_resources;
    ParticleResources& particle_resources;
    TrailResources& trail_resources;
    RenderingResources& rendering_resources;
    IGpuObjectFactory& gpu_object_factory;
    IGpuVertexArrayRenderer& gpu_vertex_array_renderer;
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
    static IGpuObjectFactory& primary_gpu_object_factory();
    static IGpuVertexArrayRenderer& primary_gpu_vertex_array_renderer();
    static int z_order();
};

}
