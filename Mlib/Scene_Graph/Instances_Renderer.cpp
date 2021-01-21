#include "Instances_Renderer.hpp"
#include <memory>

using namespace Mlib;

InstancesRendererGuard::InstancesRendererGuard(
    const std::shared_ptr<InstancesRenderer>& instances_renderer)
{
    InstancesRenderer::small_instances_renderers_.push_back(instances_renderer);
}

InstancesRendererGuard::~InstancesRendererGuard() {
    InstancesRenderer::small_instances_renderers_.pop_back();
}

std::shared_ptr<InstancesRenderer> InstancesRenderer::small_instances_renderer() {
    return small_instances_renderers_.empty() ? nullptr : small_instances_renderers_.back();
}

thread_local std::list<std::shared_ptr<InstancesRenderer>> InstancesRenderer::small_instances_renderers_;
