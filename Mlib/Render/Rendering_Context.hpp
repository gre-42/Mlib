#pragma once
#include <Mlib/Resource_Context.hpp>
#include <functional>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

struct RenderConfig;
struct RenderingContext;
class SceneNodeResources;
class ParticlesResources;
class RenderingResources;

struct RenderingContext {
    SceneNodeResources& scene_node_resources;
    ParticlesResources& particles_resources;
    std::shared_ptr<RenderingResources> rendering_resources;
    int z_order;
};

class RenderingContextGuard: public ResourceContextGuard<const RenderingContext> {
    RenderingContextGuard(const RenderingContextGuard&) = delete;
    RenderingContextGuard& operator = (const RenderingContextGuard&) = delete;
public:
    explicit RenderingContextGuard(const RenderingContext& context);
    ~RenderingContextGuard();
    static RenderingContextGuard root(
        SceneNodeResources& scene_node_resources,
        ParticlesResources& particles_resources,
        const std::string& name,
        unsigned int max_anisotropic_filtering_level,
        int z_order);
    static RenderingContextGuard layer(
        SceneNodeResources& scene_node_resources,
        ParticlesResources& particles_resources,
        const std::string& name,
        unsigned int max_anisotropic_filtering_level,
        int z_order);
};

class RenderingContextStack: public ResourceContextStack<const RenderingContext> {
    friend RenderingContextGuard;
public:
    static SceneNodeResources& primary_scene_node_resources();
    static ParticlesResources& primary_particles_resources();
    static std::shared_ptr<RenderingResources> primary_rendering_resources();
    static std::shared_ptr<RenderingResources> rendering_resources();
    static int z_order();
// private:
//     static thread_local std::list<RenderingContext> context_stack_;
};

}
