#include "Instances_Renderer.hpp"
#include <memory>

using namespace Mlib;

InstancesRendererGuard::InstancesRendererGuard(
    const std::shared_ptr<InstancesRenderers>& small_sorted_instances_renderers,
    const std::shared_ptr<InstancesRenderer>& large_instances_renderer)
{
    InstancesRenderer::small_sorted_instances_renderers_.push_back(small_sorted_instances_renderers);
    InstancesRenderer::large_instances_renderers_.push_back(large_instances_renderer);
}

InstancesRendererGuard::~InstancesRendererGuard() {
    InstancesRenderer::small_sorted_instances_renderers_.pop_back();
    InstancesRenderer::large_instances_renderers_.pop_back();
}

InstancesRenderer::~InstancesRenderer()
{}

std::shared_ptr<InstancesRenderers> InstancesRenderer::small_sorted_instances_renderers() {
    return small_sorted_instances_renderers_.empty()
        ? nullptr
        : small_sorted_instances_renderers_.back();
}

std::shared_ptr<InstancesRenderer> InstancesRenderer::large_instances_renderer() {
    return large_instances_renderers_.empty()
        ? nullptr
        : large_instances_renderers_.back();
}

thread_local std::list<std::shared_ptr<InstancesRenderers>> InstancesRenderer::small_sorted_instances_renderers_;
thread_local std::list<std::shared_ptr<InstancesRenderer>> InstancesRenderer::large_instances_renderers_;
