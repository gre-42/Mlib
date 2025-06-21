#include "Particle_Renderer.hpp"
#include <Mlib/Geometry/Material/Blending_Pass_Type.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Creator.hpp>
#include <Mlib/Render/Batch_Renderers/Particles_Instance.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <mutex>

using namespace Mlib;

ParticleRenderer::ParticleRenderer(
    ParticleResources& resources,
    ParticleType particle_type)
    : particle_type_{ particle_type }
    , resources_{ resources }
    , instances_{ [this](const VariableAndHash<std::string>& name) {
        auto res = resources_.instantiate_particles_instance(name);
        if (res->particle_type() != particle_type_) {
            THROW_OR_ABORT(
                "Error instantiating \"" + *name + "\": "
                "Particle with type \"" + particle_type_to_string(res->particle_type()) +
                "\" instantiated by \"" + particle_type_to_string(particle_type_) + '"');
        }
        return res;
      } }
    , instantiators_{ [this](const VariableAndHash<std::string>& name) {
        return resources_.instantiate_particle_creator(
            name,
            *instances_.get(resources_.get_instance_for_creator(name)));
      } }
{}

ParticleRenderer::~ParticleRenderer() {
    on_destroy.clear();
}

IParticleCreator& ParticleRenderer::get_instantiator(const VariableAndHash<std::string>& name) {
    return *instantiators_.get(name);
}

void ParticleRenderer::preload(const VariableAndHash<std::string>& name) {
    instances_.get(resources_.get_instance_for_creator(name))->preload();
}

void ParticleRenderer::advance_time(float dt, const StaticWorld& world) {
    for (auto& [_, instance] : instances_.shared()) {
        instance->move(dt, world);
    }
}

PhysicsMaterial ParticleRenderer::physics_attributes() const {
    return PhysicsMaterial::ATTR_VISIBLE;
}

RenderingStrategies ParticleRenderer::rendering_strategies() const {
    return RenderingStrategies::OBJECT;
}

bool ParticleRenderer::requires_render_pass(ExternalRenderPassType render_pass) const {
    return false;
}

BlendingPassType ParticleRenderer::required_blending_passes(ExternalRenderPassType render_pass) const {
    return BlendingPassType::LATE;
}

void ParticleRenderer::render(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const DynamicStyle* dynamic_style,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const AnimationState* animation_state,
    const ColorStyle* color_style) const
{
    for (const auto& [_, instance] : instances_.shared()) {
        if (instance->particle_type() != particle_type_) {
            THROW_OR_ABORT(
                "Particle with substrate \"" + particle_type_to_string(instance->particle_type()) +
                "\" rendered by \"" + particle_type_to_string(particle_type_) + '"');
        }
        instance->render(
            mvp,
            iv,
            lights,
            skidmarks,
            scene_graph_config,
            render_config,
            render_pass.rsd);
    }
}

ScenePos ParticleRenderer::max_center_distance2(BillboardId billboard_id) const {
    return INFINITY;
}
