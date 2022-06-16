#include "Instances_Renderer.hpp"
#include <memory>

using namespace Mlib;

InstancesRendererGuard::InstancesRendererGuard(
    const std::shared_ptr<InstancesRenderer>& small_sorted_instances_renderer,
    const std::shared_ptr<InstancesRenderer>& black_instances_renderer,
    const std::shared_ptr<InstancesRenderer>& large_instances_renderer)
{
    InstancesRenderer::small_sorted_instances_renderers_.push_back(small_sorted_instances_renderer);
    InstancesRenderer::black_small_instances_renderers_.push_back(black_instances_renderer);
    InstancesRenderer::large_instances_renderers_.push_back(black_instances_renderer);
}

InstancesRendererGuard::~InstancesRendererGuard() {
    InstancesRenderer::small_sorted_instances_renderers_.pop_back();
    InstancesRenderer::black_small_instances_renderers_.pop_back();
    InstancesRenderer::large_instances_renderers_.pop_back();
}

std::shared_ptr<InstancesRenderer> InstancesRenderer::small_sorted_instances_renderer() {
    return small_sorted_instances_renderers_.empty() ? nullptr : small_sorted_instances_renderers_.back();
}

std::shared_ptr<InstancesRenderer> InstancesRenderer::black_small_instances_renderer() {
    return black_small_instances_renderers_.empty() ? nullptr : black_small_instances_renderers_.back();
}

std::shared_ptr<InstancesRenderer> InstancesRenderer::large_instances_renderer() {
    return large_instances_renderers_.empty() ? nullptr : large_instances_renderers_.back();
}

thread_local std::list<std::shared_ptr<InstancesRenderer>> InstancesRenderer::small_sorted_instances_renderers_;
thread_local std::list<std::shared_ptr<InstancesRenderer>> InstancesRenderer::black_small_instances_renderers_;
thread_local std::list<std::shared_ptr<InstancesRenderer>> InstancesRenderer::large_instances_renderers_;
