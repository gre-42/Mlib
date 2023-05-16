#include "Instances_Renderer.hpp"

using namespace Mlib;

InstancesRendererGuard::InstancesRendererGuard(
    std::shared_ptr<InstancesRenderers> small_sorted_instances_renderers,
    std::shared_ptr<InstancesRenderer> large_instances_renderer)
: small_sorted_instances_renderers_{std::move(small_sorted_instances_renderers)},
  large_instances_renderer_{std::move(large_instances_renderer)},
  old_small_sorted_instances_renderers_{InstancesRenderer::small_sorted_instances_renderers_},
  old_large_instances_renderer_{InstancesRenderer::large_instances_renderer_}
{
    InstancesRenderer::small_sorted_instances_renderers_ = &small_sorted_instances_renderers_;
    InstancesRenderer::large_instances_renderer_ = &large_instances_renderer_;
}

InstancesRendererGuard::~InstancesRendererGuard() {
    InstancesRenderer::small_sorted_instances_renderers_ = old_small_sorted_instances_renderers_;
    InstancesRenderer::large_instances_renderer_ = old_large_instances_renderer_;
}

InstancesRenderer::~InstancesRenderer() = default;

std::shared_ptr<InstancesRenderers> InstancesRenderer::small_sorted_instances_renderers() {
    return small_sorted_instances_renderers_ == nullptr
        ? nullptr
        : *small_sorted_instances_renderers_;
}

std::shared_ptr<InstancesRenderer> InstancesRenderer::large_instances_renderer() {
    return large_instances_renderer_ == nullptr
        ? nullptr
        : *large_instances_renderer_;
}

THREAD_LOCAL(const std::shared_ptr<InstancesRenderers>*) InstancesRenderer::small_sorted_instances_renderers_ = nullptr;
THREAD_LOCAL(const std::shared_ptr<InstancesRenderer>*) InstancesRenderer::large_instances_renderer_ = nullptr;
