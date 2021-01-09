#include "Instances_Renderer.hpp"

using namespace Mlib;

InstancesRenderer* InstancesRenderer::small_instances_renderer() {
    return small_instances_renderers_.empty() ? nullptr : small_instances_renderers_.back();
}

thread_local std::list<InstancesRenderer*> InstancesRenderer::small_instances_renderers_;
