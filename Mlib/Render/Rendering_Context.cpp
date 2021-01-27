#include "Rendering_Context.hpp"
#include <Mlib/Render/Rendering_Resources.hpp>
#include <ostream>

using namespace Mlib;

thread_local std::list<RenderingContext> RenderingContextStack::context_stack_;

RenderingContextGuard::RenderingContextGuard(const RenderingContext& context) {
    RenderingContextStack::context_stack_.push_back(context);
}

RenderingContextGuard::RenderingContextGuard(
    SceneNodeResources& scene_node_resources,
    const std::string& name,
    int z_order)
: RenderingContextGuard{RenderingContext{
    .rendering_resources = std::make_shared<RenderingResources>(scene_node_resources, name),
    .z_order = z_order}}
{}

RenderingContextGuard::~RenderingContextGuard() {
    if (RenderingContextStack::context_stack_.empty()) {
        #ifdef __GNUC__
            #pragma GCC push_options
            #pragma GCC diagnostic ignored "-Wterminate"
        #endif
        throw std::runtime_error("~RenderingContextGuard but stack is empty");
        #ifdef __GNUC__
            #pragma GCC pop_options
        #endif
    }
    RenderingContextStack::context_stack_.pop_back();
}

RenderingContext RenderingContextStack::primary_rendering_context() {
    if (RenderingContextStack::context_stack_.empty()) {
        throw std::runtime_error("Primary context resources on empty stack");
    }
    return RenderingContextStack::context_stack_.front();
}

RenderingContext RenderingContextStack::rendering_context() {
    if (RenderingContextStack::context_stack_.empty()) {
        throw std::runtime_error("Secondary context on empty stack");
    }
    return RenderingContextStack::context_stack_.back();
}

std::shared_ptr<RenderingResources> RenderingContextStack::primary_rendering_resources() {
    return primary_rendering_context().rendering_resources;
}

std::shared_ptr<RenderingResources> RenderingContextStack::rendering_resources() {
    return rendering_context().rendering_resources;
}

int RenderingContextStack::z_order() {
    if (RenderingContextStack::context_stack_.empty()) {
        throw std::runtime_error("Z order on empty stack");
    }
    return RenderingContextStack::context_stack_.back().z_order;
}

std::function<std::function<void()>(std::function<void()>)>
    RenderingContextStack::generate_thread_runner(
        const RenderingContext& primary_context,
        const RenderingContext& secondary_context)
{
    return [primary_context, secondary_context](std::function<void()> f){
        return [primary_context, secondary_context, f](){
            RenderingContextGuard rrg0{primary_context};
            RenderingContextGuard rrg1{secondary_context};
            f();
        };
    };
}

void RenderingContextStack::print_stack(std::ostream& ostr) {
    ostr << "Rendering resource stack\n";
    size_t i = 0;
    for (const auto& e : context_stack_) {
        ostr << "Stack element " << i++ << '\n';
        ostr << "  z order: " << e.z_order << '\n';
        e.rendering_resources->print(ostr, 2);
    };
}
