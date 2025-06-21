#include "Particles_Instance.hpp"
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Batch_Renderers/Infer_Shader_Properties.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Clear_On_Update.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>

using namespace Mlib;

static ClearOnUpdate get_clear_on_update(ParticleType particle_type) {
    switch (particle_type) {
        case ParticleType::NONE: THROW_OR_ABORT("Partice type \"none\" does not require a particles instance");
        case ParticleType::SMOKE: return ClearOnUpdate::NO;
        case ParticleType::SKIDMARK: return ClearOnUpdate::YES;
        case ParticleType::WATER_WAVE: THROW_OR_ABORT("Water waves do not require a particles instance");
        case ParticleType::SEA_SPRAY: return ClearOnUpdate::YES;
    }
    THROW_OR_ABORT("Unknown particle substrate");
}

ParticlesInstance::ParticlesInstance(
    const std::shared_ptr<ColoredVertexArray<float>>& triangles,
    size_t max_num_instances,
    const RenderableResourceFilter& filter,
    ParticleType particle_type)
    : offset_((ScenePos)NAN)
    , dynamic_instance_buffers_{ std::make_shared<DynamicInstanceBuffers>(
        triangles->material.transformation_mode,
        max_num_instances,
        integral_cast<BillboardId>(triangles->material.billboard_atlas_instances.size()),
        get_has_per_instance_continuous_texture_layer(*triangles),
        get_clear_on_update(particle_type)) }
    , cvar_{ std::make_shared<ColoredVertexArrayResource>(triangles, dynamic_instance_buffers_) }
    , rcva_{ std::make_unique<RenderableColoredVertexArray>(RenderingContextStack::primary_rendering_resources(), cvar_, filter) }
    , filter_{ filter }
    , particle_type_{ particle_type }
{}

ParticlesInstance::~ParticlesInstance() = default;

ParticleType ParticlesInstance::particle_type() const {
    return particle_type_;
}

size_t ParticlesInstance::num_billboard_atlas_components() const {
    return dynamic_instance_buffers_->num_billboard_atlas_components();
}

void ParticlesInstance::add_particle(
    const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
    const BillboardSequence& sequence,
    const FixedArray<float, 3>& velocity,
    float air_resistance)
{
    // Lock must be above the condition for "ClearOnUpdate::YES".
    std::scoped_lock lock{ mutex_ };
    if (dynamic_instance_buffers_->tmp_length() < dynamic_instance_buffers_->capacity()) {
        if (dynamic_instance_buffers_->tmp_empty()) {
            offset_ = transformation_matrix.t;
        }
        auto trafo = TransformationMatrix<float, float, 3>{
            transformation_matrix.R,
            (transformation_matrix.t - offset_).casted<float>()};
        dynamic_instance_buffers_->append(trafo, sequence, velocity, air_resistance);
    }
}

void ParticlesInstance::move(float dt, const StaticWorld& world) {
    std::scoped_lock lock{ mutex_ };
    dynamic_instance_buffers_->move(dt, world);
}

void ParticlesInstance::preload() const {
    cvar_->preload(filter_);
}

void ParticlesInstance::render(
    const FixedArray<ScenePos, 4, 4>& vp,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderedSceneDescriptor& frame_id) const
{
    FixedArray<ScenePos, 3> offset = uninitialized;
    {
        // AperiodicLagFinder lag_finder{ "update " + std::to_string(instances->num_instances()) + " instances " + cva->name + ": ", std::chrono::milliseconds{5} };
        std::scoped_lock lock{ mutex_ };
        dynamic_instance_buffers_->update(frame_id.time_id);
        offset = offset_;
    }
    if (dynamic_instance_buffers_->num_instances() == 0) {
        return;
    }
    if (any(isnan(offset))) {
        verbose_abort("ParticlesInstance::render internal error");
    }
    TransformationMatrix<float, ScenePos, 3> m{ fixed_identity_array<float, 3>(), offset };
    rcva_->render(
        dot2d(vp, m.affine()),
        m,
        iv,
        nullptr,        // dynamic style
        lights,
        skidmarks,
        scene_graph_config,
        render_config,
        { frame_id, InternalRenderPass::PARTICLES },
        nullptr,        // animation_state,
        nullptr);       // color_style
}
