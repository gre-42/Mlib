#include "Array_Instances_Renderers.hpp"
#include <Mlib/Scene_Graph/Render/Batch_Renderers/Array_Instances_Renderer.hpp>

using namespace Mlib;

ArrayInstancesRenderers::ArrayInstancesRenderers(
    const IGpuObjectFactory& gpu_object_factory,
    IGpuVertexArrayRenderer& gpu_vertex_array_renderer)
    : gpu_object_factory_{ gpu_object_factory }
    , gpu_vertex_array_renderer_{ gpu_vertex_array_renderer }
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
        std::shared_ptr<IInstancesRenderer> v = std::dynamic_pointer_cast<IInstancesRenderer>(
            std::make_shared<ArrayInstancesRenderer>(gpu_object_factory_, gpu_vertex_array_renderer_));
        return renderers_.try_emplace(render_pass, std::move(v)).first->second;
    }
    return it->second;
}
