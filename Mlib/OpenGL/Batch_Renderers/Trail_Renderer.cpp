#include "Trail_Renderer.hpp"
#include <Mlib/OpenGL/Batch_Renderers/Trails_Instance.hpp>
#include <Mlib/OpenGL/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Trail_Storage.hpp>
#include <mutex>

using namespace Mlib;

TrailRenderer::TrailRenderer(TrailResources& resources)
    : resources_{ resources }
    , instances_{ [this](const VariableAndHash<std::string>& name) {
        return resources_.instantiate_trails_instance(name);
      } }
    , instantiators_{ [this](const VariableAndHash<std::string>& name) {
        return resources_.instantiate_storage(
            name,
            *instances_.get(resources_.get_instance_for_storage(name)));
      } }
{}

TrailRenderer::~TrailRenderer() = default;

ITrailStorage& TrailRenderer::get_storage(const VariableAndHash<std::string>& name) {
    return *instantiators_.get(name);
}

void TrailRenderer::preload(const VariableAndHash<std::string>& name) {
    #ifndef WITHOUT_GRAPHICS
    instances_.get(resources_.get_instance_for_storage(name))->preload();
    #endif
}

void TrailRenderer::move(float dt, const StaticWorld& world) {
    for (auto& [_, instance] : instances_.shared()) {
        instance->move(dt, world);
    }
}

void TrailRenderer::render(
    const FixedArray<ScenePos, 4, 4>& vp,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderedSceneDescriptor& frame_id) const
{
    for (const auto& [_, instance] : instances_.shared()) {
        instance->render(
            vp,
            iv,
            lights,
            skidmarks,
            scene_graph_config,
            render_config,
            frame_id);
    }
}
