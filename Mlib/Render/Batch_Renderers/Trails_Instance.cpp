#include "Trails_Instance.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Animated_Texture_Layer.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>

using namespace Mlib;

static std::shared_ptr<ColoredVertexArray<float>> gen_array(const std::string& texture) {
    AxisAlignedBoundingBox<float, 3> aabb{
        fixed_full<float, 3>(-INFINITY),
        fixed_full<float, 3>(INFINITY) };
    return std::make_shared<ColoredVertexArray<float>>(
        "empty_trails",
        Material{
            .blend_mode = BlendMode::CONTINUOUS,
            .textures_color = {BlendMapTexture{.texture_descriptor = {.color = {.filename = texture, .color_mode = ColorMode::RGBA}}}},
            .cull_faces = false },
        PhysicsMaterial::ATTR_VISIBLE,
        ModifierBacklog{},
        std::vector<FixedArray<ColoredVertex<float>, 4>>{},             // quads
        std::vector<FixedArray<ColoredVertex<float>, 3>>{},             // triangles
        std::vector<FixedArray<ColoredVertex<float>, 2>>{},             // lines
        std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},          // triangle_bone_weights
        std::vector<FixedArray<float, 3>>{},                            // continous_triangle_texture_layers
        std::vector<FixedArray<uint8_t, 3>>{},                          // discrete_triangle_texture_layers
        &aabb);
}

TrailsInstance::TrailsInstance(
    const std::string& texture,
    size_t max_num_segments,
    const RenderableResourceFilter& filter)
    : time_{ 0 }
    , offset_(NAN)
    , dynamic_vertex_buffers_{ std::make_shared<AnimatedTextureLayer>(max_num_segments) }
    , cvar_{ std::make_shared<ColoredVertexArrayResource>(gen_array(texture), dynamic_vertex_buffers_) }
    , rcva_{ std::make_unique<RenderableColoredVertexArray>(RenderingContextStack::primary_rendering_resources(), cvar_, filter) }
    , filter_{ filter }
{}

TrailsInstance::~TrailsInstance() = default;

void TrailsInstance::add_triangle(
    const FixedArray<ColoredVertex<double>, 3>& triangle,
    const FixedArray<float, 3>& time,
    const TrailSequence& sequence)
{
    if (dynamic_vertex_buffers_->tmp_length() < dynamic_vertex_buffers_->capacity()) {
        if (dynamic_vertex_buffers_->tmp_empty()) {
            offset_ = triangle(0).position;
        }
        auto pos = triangle.applied<ColoredVertex<float>>([this](const auto& v) { return v.translated(-offset_). template casted<float>(); });
        dynamic_vertex_buffers_->append(pos, time, sequence);
    }
}

void TrailsInstance::move(float dt) {
    dynamic_vertex_buffers_->move(dt);
    time_ += dt;
}

double TrailsInstance::time() const {
    return time_;
}

void TrailsInstance::preload() const {
    cvar_->preload(filter_);
}

void TrailsInstance::render(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const ExternalRenderPass& external_render_pass) const
{
    if (dynamic_vertex_buffers_->tmp_empty()) {
        return;
    }
    if (any(isnan(offset_))) {
        verbose_abort("TrailsInstance::render internal error");
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