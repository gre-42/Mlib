#include "Particles_Instance.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Clear_On_Update.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Interfaces/Particle_Substrate.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>

using namespace Mlib;

static ClearOnUpdate get_clear_on_update(ParticleSubstrate substrate) {
    switch (substrate) {
        case ParticleSubstrate::AIR: return ClearOnUpdate::NO;
        case ParticleSubstrate::SKIDMARK: return ClearOnUpdate::YES;
    }
    THROW_OR_ABORT("Unknown particle substrate");
}

ParticlesInstance::ParticlesInstance(
    const std::shared_ptr<ColoredVertexArray<float>>& triangles,
    size_t max_num_instances,
    const RenderableResourceFilter& filter,
    ParticleSubstrate substrate)
    : offset_(NAN)
    , dynamic_instance_buffers_{ std::make_shared<DynamicInstanceBuffers>(
        triangles->material.transformation_mode,
        max_num_instances,
        integral_cast<uint32_t>(triangles->material.billboard_atlas_instances.size()),
        get_clear_on_update(substrate)) }
    , cvar_{ std::make_shared<ColoredVertexArrayResource>(triangles, dynamic_instance_buffers_) }
    , rcva_{ std::make_unique<RenderableColoredVertexArray>(RenderingContextStack::primary_rendering_resources(), cvar_, filter) }
    , filter_{ filter }
    , substrate_{ substrate }
{}

ParticlesInstance::~ParticlesInstance() = default;

ParticleSubstrate ParticlesInstance::substrate() const {
    return substrate_;
}

void ParticlesInstance::add_particle(
    const TransformationMatrix<float, double, 3>& transformation_matrix,
    const BillboardSequence& sequence)
{
    if (dynamic_instance_buffers_->tmp_length() < dynamic_instance_buffers_->capacity()) {
        if (dynamic_instance_buffers_->tmp_empty()) {
            offset_ = transformation_matrix.t();
        }
        auto trafo = TransformationMatrix<float, float, 3>{
            transformation_matrix.R(),
            (transformation_matrix.t() - offset_).casted<float>()};
        dynamic_instance_buffers_->append(trafo, sequence);
    }
}

void ParticlesInstance::move(float dt) {
    dynamic_instance_buffers_->move(dt);
}

void ParticlesInstance::preload() const {
    cvar_->preload(filter_);
}

void ParticlesInstance::render(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass) const
{
    if (dynamic_instance_buffers_->tmp_num_instances() == 0) {
        return;
    }
    if (any(isnan(offset_))) {
        THROW_OR_ABORT("ParticlesInstance::render internal error");
    }
    TransformationMatrix<float, double, 3> m{ fixed_identity_array<float, 3>(), offset_ };
    rcva_->render(
        dot2d(vp, m.affine()),
        m,
        iv,
        lights,
        skidmarks,
        scene_graph_config,
        render_config,
        { external_render_pass, InternalRenderPass::PARTICLES },
        nullptr,        // animation_state,
        nullptr);       // color_style
}
