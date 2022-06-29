#include "Array_Instances_Renderer.hpp"
#include "Array_Instances_Renderers.hpp"

using namespace Mlib;

std::shared_ptr<InstancesRenderer> ArrayInstancesRenderers::get_instances_renderer(ExternalRenderPassType render_pass) const
{
    auto it = renderers_.find(render_pass);
    if (it == renderers_.end()) {
        std::shared_ptr<InstancesRenderer> v = std::dynamic_pointer_cast<InstancesRenderer>(std::make_shared<ArrayInstancesRenderer>());
        return renderers_.insert({render_pass, std::move(v)}).first->second;
    }
    return it->second;
}
