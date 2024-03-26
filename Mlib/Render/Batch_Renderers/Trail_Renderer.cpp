#include "Trail_Renderer.hpp"
#include <Mlib/Render/Batch_Renderers/Trails_Instance.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Storage.hpp>
#include <mutex>

using namespace Mlib;

TrailRenderer::TrailRenderer(TrailResources& resources)
    : resources_{ resources }
    , instances_{ mutex_, [&resources](const std::string& name) {
        return resources.instantiate_trails_instance(name);
      } }
    , instantiators_{ mutex_, [this, &resources](const std::string& name) {
        return resources.instantiate_storage(
            name,
            *instances_.get(resources.get_instance_for_storage(name)));
      } }
{}

TrailRenderer::~TrailRenderer() = default;

ITrailStorage& TrailRenderer::get_storage(const std::string& name) {
    return *instantiators_.get(name);
}

void TrailRenderer::preload(const std::string& name) {
    instances_.get(resources_.get_instance_for_storage(name))->preload();
}

void TrailRenderer::move(float dt, std::chrono::steady_clock::time_point time) {
    std::shared_lock lock{ mutex_ };
    for (auto& [_, instance] : instances_) {
        instance->move(dt, time);
    }
}

void TrailRenderer::render(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass) const
{
    std::shared_lock lock{ mutex_ };
    for (const auto& [_, instance] : instances_) {
        instance->render(
            vp,
            iv,
            lights,
            skidmarks,
            scene_graph_config,
            render_config,
            external_render_pass);
    }
}
