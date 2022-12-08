#include "Aggregate_Renderer.hpp"

using namespace Mlib;

AggregateRendererGuard::AggregateRendererGuard(
    const std::shared_ptr<AggregateRenderer>& small_sorted_aggregate_renderer,
    const std::shared_ptr<AggregateRenderer>& large_aggregate_renderer)
: small_sorted_aggregate_renderer_{small_sorted_aggregate_renderer},
  large_aggregate_renderer_{large_aggregate_renderer},
  old_small_sorted_aggregate_renderer_{AggregateRenderer::small_sorted_aggregate_renderer_},
  old_large_aggregate_renderer_{AggregateRenderer::large_aggregate_renderer_}
{
    AggregateRenderer::small_sorted_aggregate_renderer_ = &small_sorted_aggregate_renderer_;
    AggregateRenderer::large_aggregate_renderer_ = &large_aggregate_renderer_;
}

AggregateRendererGuard::~AggregateRendererGuard() {
    AggregateRenderer::small_sorted_aggregate_renderer_ = old_small_sorted_aggregate_renderer_;
    AggregateRenderer::large_aggregate_renderer_ = old_large_aggregate_renderer_;
}

AggregateRenderer::~AggregateRenderer() = default;

std::shared_ptr<AggregateRenderer> AggregateRenderer::small_sorted_aggregate_renderer() {
    return small_sorted_aggregate_renderer_ == nullptr
        ? nullptr
        : *small_sorted_aggregate_renderer_;
}

std::shared_ptr<AggregateRenderer> AggregateRenderer::large_aggregate_renderer() {
    return large_aggregate_renderer_ == nullptr
        ? nullptr
        : *large_aggregate_renderer_;
}

thread_local const std::shared_ptr<AggregateRenderer>* AggregateRenderer::small_sorted_aggregate_renderer_ = nullptr;
thread_local const std::shared_ptr<AggregateRenderer>* AggregateRenderer::large_aggregate_renderer_ = nullptr;
