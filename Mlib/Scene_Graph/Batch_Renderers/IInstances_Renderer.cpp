#include "IInstances_Renderer.hpp"

using namespace Mlib;

InstancesRendererGuard::InstancesRendererGuard(
    BackgroundLoop* small_instances_bg_worker,
    BackgroundLoop* large_instances_bg_worker,
    std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers,
    std::shared_ptr<IInstancesRenderer> large_instances_renderer)
    : small_sorted_instances_renderers_{std::move(small_sorted_instances_renderers)}
    , large_instances_renderer_{std::move(large_instances_renderer)}
    , old_small_instances_bg_worker_{IInstancesRenderer::small_instances_bg_worker_}
    , old_large_instances_bg_worker_{IInstancesRenderer::large_instances_bg_worker_}
    , old_small_sorted_instances_renderers_{IInstancesRenderer::small_sorted_instances_renderers_}
    , old_large_instances_renderer_{IInstancesRenderer::large_instances_renderer_}
{
    IInstancesRenderer::small_instances_bg_worker_ = small_instances_bg_worker;
    IInstancesRenderer::large_instances_bg_worker_ = large_instances_bg_worker;
    IInstancesRenderer::small_sorted_instances_renderers_ = &small_sorted_instances_renderers_;
    IInstancesRenderer::large_instances_renderer_ = &large_instances_renderer_;
}

InstancesRendererGuard::~InstancesRendererGuard() {
    IInstancesRenderer::small_instances_bg_worker_ = old_small_instances_bg_worker_;
    IInstancesRenderer::large_instances_bg_worker_ = old_large_instances_bg_worker_;
    IInstancesRenderer::small_sorted_instances_renderers_ = old_small_sorted_instances_renderers_;
    IInstancesRenderer::large_instances_renderer_ = old_large_instances_renderer_;
}

IInstancesRenderer::~IInstancesRenderer() = default;

BackgroundLoop* IInstancesRenderer::small_instances_bg_worker() {
    return small_instances_bg_worker_;
}

BackgroundLoop* IInstancesRenderer::large_instances_bg_worker() {
    return large_instances_bg_worker_;
}

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

THREAD_LOCAL(BackgroundLoop*) IInstancesRenderer::small_instances_bg_worker_ = nullptr;
THREAD_LOCAL(BackgroundLoop*) IInstancesRenderer::large_instances_bg_worker_ = nullptr;
THREAD_LOCAL(const std::shared_ptr<IInstancesRenderers>*) IInstancesRenderer::small_sorted_instances_renderers_ = nullptr;
THREAD_LOCAL(const std::shared_ptr<IInstancesRenderer>*) IInstancesRenderer::large_instances_renderer_ = nullptr;
