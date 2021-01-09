#include "Aggregate_Renderer.hpp"

using namespace Mlib;

AggregateRenderer* AggregateRenderer::small_sorted_aggregate_renderer() {
    return small_sorted_aggregate_renderers_.empty() ? nullptr : small_sorted_aggregate_renderers_.back();
}

thread_local std::list<AggregateRenderer*> AggregateRenderer::small_sorted_aggregate_renderers_;
