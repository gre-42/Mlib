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
class RenderingResources;

struct RenderingContext {
    std::shared_ptr<RenderingResources> rendering_resources;
    int z_order;
};

class RenderingContextGuard: public ResourceContextGuard<RenderingContext> {
public:
    explicit RenderingContextGuard(const RenderingContext& context);
    explicit RenderingContextGuard(
        SceneNodeResources& scene_node_resources,
        const std::string& name,
        unsigned int max_anisotropic_filtering_level,
        int z_order);
    ~RenderingContextGuard();
};

class RenderingContextStack: public ResourceContextStack<RenderingContext> {
    friend RenderingContextGuard;
public:
    static std::shared_ptr<RenderingResources> primary_rendering_resources();
    static std::shared_ptr<RenderingResources> rendering_resources();
    static int z_order();
    static void print_stack(std::ostream& ostr);
// private:
//     static thread_local std::list<RenderingContext> context_stack_;
};

}
