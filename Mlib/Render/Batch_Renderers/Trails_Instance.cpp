#include "Trails_Instance.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Animated_Texture_Layer.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>

using namespace Mlib;

static std::shared_ptr<ColoredVertexArray<float>> gen_array(
    const VariableAndHash<std::string>& texture,
    const Shading& shading,
    const std::vector<float>& continuous_layer_x,
    const std::vector<float>& continuous_layer_y)
{
    auto aabb = ExtremalAxisAlignedBoundingBox<float, 3>{ ExtremalBoundingVolume::FULL };
    auto bounding_sphere = ExtremalBoundingSphere<float, 3>{ ExtremalBoundingVolume::FULL };
    return std::make_shared<ColoredVertexArray<float>>(
        "empty_trails",
        Material{
            .blend_mode = BlendMode::CONTINUOUS,
            .textures_color = {BlendMapTexture{.texture_descriptor = {.color = ColormapWithModifiers{
                .filename = texture,
                .color_mode = ColorMode::RGBA,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS_2D,
                .depth_interpolation = InterpolationMode::LINEAR}.compute_hash()}}},
            .continuous_layer_x = continuous_layer_x,
            .continuous_layer_y = continuous_layer_y,
            .magnifying_interpolation_mode = InterpolationMode::LINEAR,
            .cull_faces = false,
            .shading = shading },
        Morphology{
            .physics_material = PhysicsMaterial::ATTR_VISIBLE
        },
        ModifierBacklog{},
        UUVector<FixedArray<ColoredVertex<float>, 4>>{},            // quads
        UUVector<FixedArray<ColoredVertex<float>, 3>>{},            // triangles
        UUVector<FixedArray<ColoredVertex<float>, 2>>{},            // lines
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},         // triangle_bone_weights
        UUVector<FixedArray<float, 3>>{},                           // continous_triangle_texture_layers
        UUVector<FixedArray<uint8_t, 3>>{},                         // discrete_triangle_texture_layers
        std::vector<UUVector<FixedArray<float, 3, 2>>>{},           // uv1
        std::vector<UUVector<FixedArray<float, 3>>>{},              // cweight
        UUVector<FixedArray<float, 3>>{},                           // alpha
        UUVector<FixedArray<float, 4>>{},                           // interiormap_uvmap
        &aabb,
        &bounding_sphere);
}

TrailsInstance::TrailsInstance(
    const VariableAndHash<std::string>& texture,
    const Shading& shading,
    const std::vector<float>& continuous_layer_x,
    const std::vector<float>& continuous_layer_y,
    size_t max_num_segments,
    const RenderableResourceFilter& filter)
    : offset_((ScenePos)NAN)
    , dynamic_vertex_buffers_{ std::make_shared<AnimatedTextureLayer>(max_num_segments) }
    , cvar_{ std::make_shared<ColoredVertexArrayResource>(gen_array(texture, shading, continuous_layer_x, continuous_layer_y), dynamic_vertex_buffers_) }
    , rcva_{ std::make_unique<RenderableColoredVertexArray>(RenderingContextStack::primary_rendering_resources(), cvar_, filter) }
    , filter_{ filter }
{}

TrailsInstance::~TrailsInstance() = default;

void TrailsInstance::add_triangle(
    const FixedArray<ColoredVertex<ScenePos>, 3>& triangle,
    const FixedArray<float, 3>& time,
    const TrailSequence& sequence)
{
    if (dynamic_vertex_buffers_->tmp_length() < dynamic_vertex_buffers_->capacity()) {
        std::scoped_lock lock{ mutex_ };
        if (dynamic_vertex_buffers_->tmp_empty()) {
            offset_ = triangle(0).position;
        }
        auto pos = triangle.applied<ColoredVertex<float>>([this](const auto& v) { return v.translated(-offset_). template casted<float>(); });
        dynamic_vertex_buffers_->append(pos, time, sequence);
    }
}

void TrailsInstance::move(float dt, const StaticWorld& world) {
    std::scoped_lock lock{ mutex_ };
    dynamic_vertex_buffers_->move(dt, world);
}

std::chrono::steady_clock::time_point TrailsInstance::time() const {
    return dynamic_vertex_buffers_->time();
}

void TrailsInstance::preload() const {
    cvar_->preload(filter_);
}

void TrailsInstance::render(
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
        dynamic_vertex_buffers_->update(frame_id.external_render_pass.time);
        offset = offset_;
    }
    if (dynamic_vertex_buffers_->ntriangles() == 0) {
        return;
    }
    if (any(isnan(offset_))) {
        verbose_abort("TrailsInstance::render internal error");
    }
    TransformationMatrix<float, ScenePos, 3> m{ fixed_identity_array<float, 3>(), offset_ };
    rcva_->render(
        dot2d(vp, m.affine()),
        m,
        iv,
        nullptr,        // dynamic_style
        lights,
        skidmarks,
        scene_graph_config,
        render_config,
        { frame_id, InternalRenderPass::PARTICLES },
        nullptr,        // animation_state,
        nullptr);       // color_style
}
