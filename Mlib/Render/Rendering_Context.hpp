#pragma once
#include <functional>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

struct RenderingContext;
class SceneNodeResources;
class RenderingResources;

class RenderingContextGuard {
public:
    explicit RenderingContextGuard(const RenderingContext& context);
    explicit RenderingContextGuard(
        SceneNodeResources& scene_node_resources,
        const std::string& name,
        int z_order);
    ~RenderingContextGuard();
};

struct RenderingContext {
    std::shared_ptr<RenderingResources> rendering_resources;
    int z_order;
};

class RenderingContextStack {
    friend RenderingContextGuard;
public:
    static RenderingContext primary_rendering_context();
    static RenderingContext rendering_context();
    static std::shared_ptr<RenderingResources> primary_rendering_resources();
    static std::shared_ptr<RenderingResources> rendering_resources();
    static int z_order();
    static std::function<std::function<void()>(std::function<void()>)>
        generate_thread_runner(
            const RenderingContext& primary_context,
            const RenderingContext& secondary_context);
    static void print_stack(std::ostream& ostr);
private:
    static thread_local std::list<RenderingContext> context_stack_;
};

std::ostream& operator << (std::ostream& ostr, const RenderingContext& r);

}
