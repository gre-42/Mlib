#include "Aggregate_Renderer.hpp"

using namespace Mlib;

AggregateRendererGuard::AggregateRendererGuard(
    const std::shared_ptr<AggregateRenderer>& small_sorted_aggregate_renderer,
    const std::shared_ptr<AggregateRenderer>& large_aggregate_renderer)
{
    AggregateRenderer::small_sorted_aggregate_renderers_.push_back(small_sorted_aggregate_renderer);
    AggregateRenderer::large_aggregate_renderers_.push_back(large_aggregate_renderer);
}

AggregateRendererGuard::~AggregateRendererGuard() {
    AggregateRenderer::small_sorted_aggregate_renderers_.pop_back();
    AggregateRenderer::large_aggregate_renderers_.pop_back();
}

std::shared_ptr<AggregateRenderer> AggregateRenderer::small_sorted_aggregate_renderer() {
    return small_sorted_aggregate_renderers_.empty() ? nullptr : small_sorted_aggregate_renderers_.back();
}

std::shared_ptr<AggregateRenderer> AggregateRenderer::large_aggregate_renderer() {
    return large_aggregate_renderers_.empty() ? nullptr : large_aggregate_renderers_.back();
}

thread_local std::list<std::shared_ptr<AggregateRenderer>> AggregateRenderer::small_sorted_aggregate_renderers_;
thread_local std::list<std::shared_ptr<AggregateRenderer>> AggregateRenderer::large_aggregate_renderers_;
