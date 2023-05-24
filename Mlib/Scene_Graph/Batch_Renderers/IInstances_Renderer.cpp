#include "IInstances_Renderer.hpp"

using namespace Mlib;

InstancesRendererGuard::InstancesRendererGuard(
    std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers,
    std::shared_ptr<IInstancesRenderer> large_instances_renderer)
: small_sorted_instances_renderers_{std::move(small_sorted_instances_renderers)},
  large_instances_renderer_{std::move(large_instances_renderer)},
  old_small_sorted_instances_renderers_{IInstancesRenderer::small_sorted_instances_renderers_},
  old_large_instances_renderer_{IInstancesRenderer::large_instances_renderer_}
{
    IInstancesRenderer::small_sorted_instances_renderers_ = &small_sorted_instances_renderers_;
    IInstancesRenderer::large_instances_renderer_ = &large_instances_renderer_;
}

InstancesRendererGuard::~InstancesRendererGuard() {
    IInstancesRenderer::small_sorted_instances_renderers_ = old_small_sorted_instances_renderers_;
    IInstancesRenderer::large_instances_renderer_ = old_large_instances_renderer_;
}

IInstancesRenderer::~IInstancesRenderer() = default;

std::shared_ptr<IInstancesRenderers> IInstancesRenderer::small_sorted_instances_renderers() {
    return small_sorted_instances_renderers_ == nullptr
        ? nullptr
        : *small_sorted_instances_renderers_;
}

std::shared_ptr<IInstancesRenderer> IInstancesRenderer::large_instances_renderer() {
    return large_instances_renderer_ == nullptr
        ? nullptr
        : *large_instances_renderer_;
}

THREAD_LOCAL(const std::shared_ptr<IInstancesRenderers>*) IInstancesRenderer::small_sorted_instances_renderers_ = nullptr;
THREAD_LOCAL(const std::shared_ptr<IInstancesRenderer>*) IInstancesRenderer::large_instances_renderer_ = nullptr;
