#include "Particles_Instance.hpp"
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>

using namespace Mlib;

ParticlesInstance::ParticlesInstance(
    const std::shared_ptr<ColoredVertexArray<float>>& triangles,
    TransformationMode transformation_mode,
    size_t max_num_instances,
    uint32_t num_billboard_atlas_components,
    const std::string& name)
: dynamic_instance_buffers_{std::make_shared<DynamicInstanceBuffers>(
    transformation_mode,
    integral_cast<GLsizei>(max_num_instances),
    num_billboard_atlas_components,
    name)},
  cvar_{std::make_shared<ColoredVertexArrayResource>(
    triangles,
    dynamic_instance_buffers_)},
  rcva_{std::make_unique<RenderableColoredVertexArray>(cvar_, RenderableResourceFilter{})}
{}

ParticlesInstance::~ParticlesInstance() = default;

void ParticlesInstance::add_particle(
    const TransformationMatrix<float, double, 3>& transformation_matrix,
    const BillboardSequence& sequence)
{
    if (dynamic_instance_buffers_->length() < dynamic_instance_buffers_->capacity()) {
        if (dynamic_instance_buffers_->empty()) {
            offset_ = transformation_matrix.t();
        }
        auto trafo = TransformationMatrix<float, float, 3>{
            transformation_matrix.R(),
            (transformation_matrix.t() - offset_).casted<float>()};
        dynamic_instance_buffers_->append(trafo, sequence);
    }
}

void ParticlesInstance::render(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass) const
{
    TransformationMatrix<float, double, 3> m{fixed_identity_array<float, 3>(), offset_};
    rcva_->render(
        dot2d(vp, m.affine()),
        m,
        iv,
        lights,
        scene_graph_config,
        render_config,
        {external_render_pass, InternalRenderPass::AGGREGATE},
        nullptr,        // animation_state,
        nullptr);       // color_style
}
