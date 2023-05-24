#include "Array_Instances_Renderer.hpp"
#include "Array_Instances_Renderers.hpp"

using namespace Mlib;

void ArrayInstancesRenderers::invalidate() {
    for (auto& [_, renderer] : renderers_) {
        renderer->invalidate();
    }
}

std::shared_ptr<IInstancesRenderer> ArrayInstancesRenderers::get_instances_renderer(ExternalRenderPassType render_pass) const
{
    auto it = renderers_.find(render_pass);
    if (it == renderers_.end()) {
        std::shared_ptr<IInstancesRenderer> v = std::dynamic_pointer_cast<IInstancesRenderer>(std::make_shared<ArrayInstancesRenderer>());
        return renderers_.insert({render_pass, std::move(v)}).first->second;
    }
    return it->second;
}
