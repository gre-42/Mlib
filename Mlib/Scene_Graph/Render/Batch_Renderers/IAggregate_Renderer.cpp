#include "IAggregate_Renderer.hpp"

using namespace Mlib;

AggregateRendererGuard::AggregateRendererGuard(
    BackgroundLoop* small_aggregate_bg_worker,
    BackgroundLoop* large_aggregate_bg_worker,
    std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer,
    std::shared_ptr<IAggregateRenderer> large_aggregate_renderer)
    : small_sorted_aggregate_renderer_{std::move(small_sorted_aggregate_renderer)}
    , large_aggregate_renderer_{std::move(large_aggregate_renderer)}
    , old_small_aggregate_bg_worker_{IAggregateRenderer::small_aggregate_bg_worker_}
    , old_large_aggregate_bg_worker_{IAggregateRenderer::large_aggregate_bg_worker_}
    , old_small_sorted_aggregate_renderer_{IAggregateRenderer::small_sorted_aggregate_renderer_}
    , old_large_aggregate_renderer_{IAggregateRenderer::large_aggregate_renderer_}
{
    IAggregateRenderer::small_aggregate_bg_worker_ = small_aggregate_bg_worker;
    IAggregateRenderer::large_aggregate_bg_worker_ = large_aggregate_bg_worker;
    IAggregateRenderer::small_sorted_aggregate_renderer_ = &small_sorted_aggregate_renderer_;
    IAggregateRenderer::large_aggregate_renderer_ = &large_aggregate_renderer_;
}

AggregateRendererGuard::~AggregateRendererGuard() {
    IAggregateRenderer::small_aggregate_bg_worker_ = old_small_aggregate_bg_worker_;
    IAggregateRenderer::large_aggregate_bg_worker_ = old_large_aggregate_bg_worker_;
    IAggregateRenderer::small_sorted_aggregate_renderer_ = old_small_sorted_aggregate_renderer_;
    IAggregateRenderer::large_aggregate_renderer_ = old_large_aggregate_renderer_;
}

IAggregateRenderer::~IAggregateRenderer() = default;

BackgroundLoop* IAggregateRenderer::small_aggregate_bg_worker() {
    return small_aggregate_bg_worker_;
}

BackgroundLoop* IAggregateRenderer::large_aggregate_bg_worker() {
    return large_aggregate_bg_worker_;
}

std::shared_ptr<IAggregateRenderer> IAggregateRenderer::small_sorted_aggregate_renderer() {
    return small_sorted_aggregate_renderer_ == nullptr
        ? nullptr
        : *small_sorted_aggregate_renderer_;
}

std::shared_ptr<IAggregateRenderer> IAggregateRenderer::large_aggregate_renderer() {
    return large_aggregate_renderer_ == nullptr
        ? nullptr
        : *large_aggregate_renderer_;
}

THREAD_LOCAL(BackgroundLoop*) IAggregateRenderer::small_aggregate_bg_worker_ = nullptr;
THREAD_LOCAL(BackgroundLoop*) IAggregateRenderer::large_aggregate_bg_worker_ = nullptr;
THREAD_LOCAL(const std::shared_ptr<IAggregateRenderer>*) IAggregateRenderer::small_sorted_aggregate_renderer_ = nullptr;
THREAD_LOCAL(const std::shared_ptr<IAggregateRenderer>*) IAggregateRenderer::large_aggregate_renderer_ = nullptr;
