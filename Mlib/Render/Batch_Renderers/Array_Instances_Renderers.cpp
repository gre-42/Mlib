#include "Array_Instances_Renderer.hpp"
#include "Array_Instances_Renderers.hpp"

using namespace Mlib;

ArrayInstancesRenderers::ArrayInstancesRenderers(RenderingResources& rendering_resources)
    : rendering_resources_{ rendering_resources }
{}

void ArrayInstancesRenderers::invalidate() {
    for (auto& [_, renderer] : renderers_) {
        renderer->invalidate();
    }
}

std::shared_ptr<IInstancesRenderer> ArrayInstancesRenderers::get_instances_renderer(ExternalRenderPassType render_pass) const
{
    auto it = renderers_.find(render_pass);
    if (it == renderers_.end()) {
        std::shared_ptr<IInstancesRenderer> v = std::dynamic_pointer_cast<IInstancesRenderer>(std::make_shared<ArrayInstancesRenderer>(rendering_resources_));
        return renderers_.insert({render_pass, std::move(v)}).first->second;
    }
    return it->second;
}
