#include "Aggregate_Renderer.hpp"

using namespace Mlib;

AggregateRendererGuard::AggregateRendererGuard(
    const std::shared_ptr<AggregateRenderer>& aggregate_renderer)
{
    AggregateRenderer::small_sorted_aggregate_renderers_.push_back(aggregate_renderer);
}

AggregateRendererGuard::~AggregateRendererGuard() {
    AggregateRenderer::small_sorted_aggregate_renderers_.pop_back();
}

std::shared_ptr<AggregateRenderer> AggregateRenderer::small_sorted_aggregate_renderer() {
    return small_sorted_aggregate_renderers_.empty() ? nullptr : small_sorted_aggregate_renderers_.back();
}

thread_local std::list<std::shared_ptr<AggregateRenderer>> AggregateRenderer::small_sorted_aggregate_renderers_;
