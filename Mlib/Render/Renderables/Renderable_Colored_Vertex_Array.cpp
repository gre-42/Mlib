#include "Renderable_Colored_Vertex_Array.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Frustum3.hpp>
#include <Mlib/Geometry/Material/Interior_Texture_Set.hpp>
#include <Mlib/Geometry/Material_Features.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Hash.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/Batch_Renderers/Infer_Shader_Properties.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Frame_Index_From_Animation_Time.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Instance_Handles/Wrap_Mode.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IInstance_Buffers.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IVertex_Data.hpp>
#include <Mlib/Render/Toggle_Benchmark_Rendering.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/Task_Location.hpp>
#include <Mlib/Scene_Graph/Culling/Frustum_Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Culling/Instances_Are_Visible.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Dynamic_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Instances/Large_Instances_Queue.hpp>
#include <Mlib/Scene_Graph/Instances/Small_Instances_Queues.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <climits>
#include <unordered_map>

// #undef LOG_FUNCTION
// #undef LOG_INFO
// #define LOG_FUNCTION(msg) ::Mlib::Log log(msg)
// #define LOG_INFO(msg) log.info(msg)
// #define LOG_INFO(msg) linfo() << msg

using namespace Mlib;

static const auto dirtmap_name = VariableAndHash<std::string>{ "dirtmap" };

struct TextureIndexCalculator {
    size_t ntextures_color;
    size_t ntextures_alpha;
    size_t ntextures_lightmaps;
    size_t ntextures_filtered_skidmarks;
    size_t ntextures_normal;
    size_t ntextures_dirt;
    size_t ntextures_interior;
    size_t ntextures_reflection;
    size_t ntextures_specular;

    size_t id_color(size_t i) const {
        assert_true(i < ntextures_color);
        return id_color_(i);
    }
    size_t id_alpha(size_t i) const {
        assert_true(i < ntextures_alpha);
        return id_alpha_(i);
    }
    size_t id_light(size_t i) const {
        assert_true(i < ntextures_lightmaps);
        return id_light_(i);
    }
    size_t id_skidmark(size_t i) const {
        assert_true(i < ntextures_filtered_skidmarks);
        return id_skidmark_(i);
    }
    size_t id_normal(size_t i) const {
        assert_true(i < ntextures_normal);
        return id_normal_(i);
    }
    size_t id_dirt(size_t i) const {
        assert_true(i < ntextures_dirt);
        return id_dirt_(i);
    }
    size_t id_reflection() const {
        assert_true(0 < ntextures_reflection);
        return id_reflection_();
    }
    size_t id_interior(size_t i) const {
        assert_true(i < ntextures_interior);
        return id_interior_(i);
    }
    size_t id_specular() const {
        assert_true(0 < ntextures_specular);
        return id_specular_();
    }

    size_t id_color_(size_t i) const {
        return i;
    }
    size_t id_alpha_(size_t i) const {
        return id_color_(0) + ntextures_color + i;
    }
    size_t id_light_(size_t i) const {
        return id_alpha_(0) + ntextures_alpha + i;
    }
    size_t id_skidmark_(size_t i) const {
        return id_light_(0) + ntextures_lightmaps + i;
    }
    size_t id_normal_(size_t i) const {
        return id_skidmark_(0) + ntextures_filtered_skidmarks + i;
    }
    size_t id_dirt_(size_t i) const {
        return id_normal_(0) + ntextures_normal + i;
    }
    size_t id_reflection_() const {
        return id_dirt_(0) + ntextures_dirt;
    }
    size_t id_interior_(size_t i) const {
        return id_reflection_() + ntextures_reflection + i;
    }
    size_t id_specular_() const {
        return id_interior_(0) + ntextures_interior;
    }
};

std::ostream& operator << (std::ostream& ostr, const TextureIndexCalculator& tic) {
    ostr << "ntextures_color: " << tic.ntextures_color << '\n';
    ostr << "ntextures_alpha: " << tic.ntextures_alpha << '\n';
    ostr << "ntextures_lightmaps: " << tic.ntextures_lightmaps << '\n';
    ostr << "ntextures_filtered_skidmarks: " << tic.ntextures_filtered_skidmarks << '\n';
    ostr << "ntextures_normal: " << tic.ntextures_normal << '\n';
    ostr << "ntextures_dirt: " << tic.ntextures_dirt << '\n';
    ostr << "ntextures_interior: " << tic.ntextures_interior << '\n';
    ostr << "ntextures_reflection: " << tic.ntextures_reflection << '\n';
    ostr << "ntextures_specular: " << tic.ntextures_specular << '\n';
    return ostr;
}

static const int CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED = INT_MAX;
static const int CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING = INT_MIN;

RenderableColoredVertexArray::RenderableColoredVertexArray(
    RenderingResources& rendering_resources,
    const std::shared_ptr<const ColoredVertexArrayResource>& rcva,
    const RenderableResourceFilter& renderable_resource_filter)
    : rcva_{ rcva }
    , continuous_blending_z_order_{ CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED }
    , secondary_rendering_resources_{ rendering_resources }
    , aabb_{ ExtremalBoundingVolume::EMPTY }
    , bounding_sphere_{ ExtremalBoundingVolume::EMPTY }
{
#ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
#endif
    requires_blending_pass_ = false;
    auto add_cvas = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
    {
        for (const auto& [i, t] : enumerate(cvas)) {
            if (renderable_resource_filter.matches(i, *t)) {
                if (any(t->morphology.physics_material & PhysicsMaterial::ATTR_VISIBLE)) {
                    if ((t->material.aggregate_mode == AggregateMode::NONE) ||
                        (rcva->instances_ != nullptr))
                    {
                        if constexpr (std::is_same_v<TPos, float>) {
                            aggregate_off_.push_back(t);
                            required_occluder_passes_.insert(t->material.occluder_pass);
                        } else {
                            THROW_OR_ABORT("Instances and aggregate=off require single precision (material: " + t->material.identifier() + ')');
                        }
                    } else if (t->material.aggregate_mode == AggregateMode::ONCE) {
                        if constexpr (std::is_same_v<TPos, CompressedScenePos>) {
                            aggregate_once_.push_back(t);
                        } else {
                            THROW_OR_ABORT("Aggregate=once requires double precision (material: " + t->material.identifier() + ')');
                        }
                    } else if (t->material.aggregate_mode == AggregateMode::SORTED_CONTINUOUSLY) {
                        if constexpr (std::is_same_v<TPos, float>) {
                            saggregate_sorted_continuously_.push_back(t);
                        } else {
                            daggregate_sorted_continuously_.push_back(t);
                        }
                    } else if (t->material.aggregate_mode == AggregateMode::INSTANCES_ONCE) {
                        if constexpr (std::is_same_v<TPos, float>) {
                            instances_once_.push_back(t);
                        } else {
                            THROW_OR_ABORT("Aggregate=instances_once requires single precision (material: " + t->material.identifier() + ')');
                        }
                    } else if (t->material.aggregate_mode == AggregateMode::INSTANCES_SORTED_CONTINUOUSLY) {
                        if constexpr (std::is_same_v<TPos, float>) {
                            instances_sorted_continuously_.push_back(t);
                        } else {
                            THROW_OR_ABORT("Aggregate=instances_sorted_continuously requires single precision (material: " + t->material.identifier() + ')');
                        }
                    } else {
                        THROW_OR_ABORT("Unknown aggregate mode");
                    }
                    if ((t->material.continuous_blending_z_order == CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED) ||
                        (t->material.continuous_blending_z_order == CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING))
                    {
                        THROW_OR_ABORT("Unsupported \"continuous_blending_z_order\" value");
                    }
                    if (continuous_blending_z_order_ != CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING) {
                        if (any(t->material.blend_mode & BlendMode::ANY_CONTINUOUS) &&
                            (t->material.aggregate_mode == AggregateMode::NONE))
                        {
                            requires_blending_pass_ = true;
                            if (continuous_blending_z_order_ == CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED) {
                                continuous_blending_z_order_ = t->material.continuous_blending_z_order;
                            } else if (continuous_blending_z_order_ != t->material.continuous_blending_z_order) {
                                continuous_blending_z_order_ = CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING;
                            }
                        }
                    }
                }
                if (any(t->morphology.physics_material & PhysicsMaterial::ATTR_COLLIDE)) {
                    if constexpr (std::is_same_v<TPos, float>) {
                        sphysics_.push_back(t);
                    } else if constexpr (std::is_same_v<TPos, CompressedScenePos>) {
                        dphysics_.push_back(t);
                    } else {
                        THROW_OR_ABORT("Unknown physics precision");
                    }
                }
            }
        }
    };
    if (rcva->triangles_res_->scvas.empty() &&
        rcva->triangles_res_->dcvas.empty())
    {
        THROW_OR_ABORT("RenderableColoredVertexArray received no arrays");
    }
    add_cvas(rcva->triangles_res_->scvas);
    add_cvas(rcva->triangles_res_->dcvas);
    if (aggregate_off_.empty() &&
        aggregate_once_.empty() &&
        saggregate_sorted_continuously_.empty() &&
        daggregate_sorted_continuously_.empty() &&
        instances_once_.empty() &&
        instances_sorted_continuously_.empty() &&
        sphysics_.empty() &&
        dphysics_.empty())
    {
        THROW_OR_ABORT(
            "Filter did not match a single array.\n" +
            (std::stringstream() << renderable_resource_filter).str());
    }

    for (auto& cva : aggregate_off_) {
        aabb_.extend(cva->aabb().casted<CompressedScenePos>());
        bounding_sphere_.extend(cva->bounding_sphere().casted<CompressedScenePos>());
    }
    for (auto& cva : aggregate_once_) {
        aabb_.extend(cva->aabb());
        bounding_sphere_.extend(cva->bounding_sphere());
    }
    for (auto& cva : saggregate_sorted_continuously_) {
        aabb_.extend(cva->aabb().casted<CompressedScenePos>());
        bounding_sphere_.extend(cva->bounding_sphere().casted<CompressedScenePos>());
    }
    for (auto& cva : daggregate_sorted_continuously_) {
        aabb_.extend(cva->aabb());
        bounding_sphere_.extend(cva->bounding_sphere());
    }
}

RenderableColoredVertexArray::~RenderableColoredVertexArray() = default;

UUVector<OffsetAndQuaternion<float, float>> RenderableColoredVertexArray::calculate_absolute_bone_transformations(const AnimationState* animation_state) const
{
    TIME_GUARD_DECLARE(time_guard, "calculate_absolute_bone_transformations", "calculate_absolute_bone_transformations");
    if (!rcva_->triangles_res_->bone_indices.empty()) {
        if (animation_state == nullptr) {
            THROW_OR_ABORT("Animation without animation state");
        }
        auto get_abt = [this](const VariableAndHash<std::string>& animation_name, float time) {
            if (animation_name->empty()) {
                THROW_OR_ABORT("Animation frame has no name");
            }
            if (std::isnan(time)) {
                THROW_OR_ABORT("Vertex array loop time is NAN");
            }
            auto poses = rcva_->scene_node_resources_.get_relative_poses(
                animation_name,
                time);
            UUVector<OffsetAndQuaternion<float, float>> ms = rcva_->triangles_res_->vectorize_joint_poses(poses);
            UUVector<OffsetAndQuaternion<float, float>> absolute_bone_transformations = rcva_->triangles_res_->skeleton->rebase_to_initial_absolute_transform(ms);
            if (absolute_bone_transformations.size() != rcva_->triangles_res_->bone_indices.size()) {
                THROW_OR_ABORT("Number of bone indices differs from number of quaternions");
            }
            return absolute_bone_transformations;
        };
        if (animation_state->aperiodic_animation_frame.active()) {
            return get_abt(animation_state->aperiodic_skelletal_animation_name, animation_state->aperiodic_animation_frame.time());
        } else {
            return get_abt(animation_state->periodic_skelletal_animation_name, animation_state->periodic_skelletal_animation_frame.time());
        }
    } else {
        return {};
    }
}

struct IdAndTexture {
    size_t id;
    const ITextureHandle& texture;
};

void RenderableColoredVertexArray::render_cva(
    const std::shared_ptr<ColoredVertexArray<float>>& cva,
    const UUVector<OffsetAndQuaternion<float, float>>& absolute_bone_transformations,
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
    // AperiodicLagFinder lag_finder{ "render_cva " + cva->name + ": ", std::chrono::milliseconds{5} };
    LOG_FUNCTION("render_cva");
    LOG_INFO("RenderableColoredVertexArray::render_cva " + cva->identifier());
    TIME_GUARD_DECLARE(time_guard, "render_cva", cva->identifier());
    // lerr() << external_render_pass_type_to_string(render_pass.external.pass) << " " << cva->identifier();
    // if (rcva_->instances_ != nullptr) {
    //     lerr() << ", #inst: " << rcva_->instances_->at(cva.get())->num_instances();
    // }
    // This check passes because the arrays are filtered in the constructor.
    assert_true((cva->material.aggregate_mode == AggregateMode::NONE) || (rcva_->instances_ != nullptr));
    if (render_pass.internal == InternalRenderPass::INITIAL && any(cva->material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
        // lerr() << ", skipped (0)";
        return;
    }
    if (render_pass.internal == InternalRenderPass::BLENDED && !any(cva->material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
        // lerr() << ", skipped (1)";
        return;
    }
    // This is now done in the VisibilityCheck class.
    // if (render_pass.external.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE && render_pass.external.black_node_name.empty() && cva->material.occluder_pass == OccluderType::OFF) {
    //     return;
    // }
    std::shared_ptr<IInstanceBuffers> instances;
    auto mvp_f = mvp.casted<float>();
    VisibilityCheck vc{ mvp_f };
    if (rcva_->instances_ == nullptr) {
        FrustumVisibilityCheck fvc{vc};
        if (!fvc.is_visible(cva->name.full_name(), cva->material, cva->morphology, BILLBOARD_ID_NONE, scene_graph_config, render_pass.external.pass, cva->aabb()))
        {
            // lerr() << ", skipped (2)";
            return;
        }
    } else {
        if (!instances_are_visible(cva->material, render_pass.external.pass)) {
            return;
        }
        instances = rcva_->instances_->at(cva.get());
        if (instances->num_instances() == 0) {
            return;
        }
    }
    // lerr();

    std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>> filtered_lights;
    std::vector<size_t> lightmap_indices;
    std::vector<size_t> light_noshadow_indices;
    std::vector<size_t> light_shadow_indices;
    std::vector<size_t> black_shadow_indices;
    std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>> filtered_skidmarks;
    bool is_lightmap = any(render_pass.external.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK);
    FixedArray<float, 3> fog_emissive = fixed_zeros<float, 3>();
    FixedArray<float, 3> emissive = fixed_zeros<float, 3>();
    FixedArray<float, 3> ambient = fixed_zeros<float, 3>();
    FixedArray<float, 3> diffuse = fixed_zeros<float, 3>();
    FixedArray<float, 3> specular = fixed_zeros<float, 3>();
    float specular_exponent = 0.f;
    FixedArray<float, 3> fresnel_ambient = fixed_zeros<float, 3>();
    if (!is_lightmap) {
        emissive = color_style && !color_style->emissive.all_equal(-1.f)
            ? color_style->emissive
            : cva->material.shading.emissive;
        ambient = color_style && !color_style->ambient.all_equal(-1.f) ? color_style->ambient * cva->material.shading.ambient : cva->material.shading.ambient;
        diffuse = color_style && !color_style->diffuse.all_equal(-1.f) ? color_style->diffuse * cva->material.shading.diffuse : cva->material.shading.diffuse;
        specular = color_style && !color_style->specular.all_equal(-1.f) ? color_style->specular * cva->material.shading.specular : cva->material.shading.specular;
        specular_exponent = color_style && (color_style->specular_exponent != -1.f) ? color_style->specular_exponent : cva->material.shading.specular_exponent;
        fresnel_ambient = color_style && !color_style->fresnel_ambient.all_equal(-1.f)
            ? color_style->fresnel_ambient * cva->material.shading.fresnel.ambient
            : cva->material.shading.fresnel.ambient;
    } else {
        emissive = 1.f;
    }
    if (!is_lightmap) {
        filtered_lights.reserve(lights.size());
        light_noshadow_indices.reserve(lights.size());
        light_shadow_indices.reserve(lights.size());
        black_shadow_indices.reserve(lights.size());
        lightmap_indices.reserve(lights.size());
        for (const auto& tl : lights) {
            const auto& l = *tl.second;
            // By this definition, objects are occluded/lighted (occluded_pass)
            // by several shadowmaps/lightmaps (low-resolution and high-resolution shadowmaps).
            // The occluder_pass is checked in the "VisibilityCheck" class.
            if (cva->material.occluded_pass < l.shadow_render_pass) {
                continue;
            }
            bool light_emits_colors = l.emits_colors();
            bool light_casts_shadows = any(l.shadow_render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK);

            if (!light_casts_shadows) {
                if (!light_emits_colors) {
                    continue;
                }
                if (all(l.ambient * ambient == 0.f) &&
                    all(l.diffuse * diffuse == 0.f) &&
                    all(l.specular * specular == 0.f) &&
                    all(l.fresnel_ambient * fresnel_ambient == 0.f))
                {
                    continue;
                }
            }
            size_t i = filtered_lights.size();
            filtered_lights.push_back(tl);
            if (light_emits_colors) {
                if (light_casts_shadows) {
                    lightmap_indices.push_back(i);
                    light_shadow_indices.push_back(i);
                    if (any(cva->material.occluded_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK) &&
                        (l.lightmap_color == nullptr))
                    {
                        THROW_OR_ABORT("Light with color shadows has no color texture");
                    }
                    if (any(cva->material.occluded_pass & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK) &&
                        (l.lightmap_depth == nullptr))
                    {
                        THROW_OR_ABORT("Light with depth shadows has no depth texture");
                    }
                } else {
                    light_noshadow_indices.push_back(i);
                    if (l.lightmap_color != nullptr) {
                        THROW_OR_ABORT("Light without shadow has a color texture");
                    }
                    if (l.lightmap_depth != nullptr) {
                        THROW_OR_ABORT("Light without shadow has a depth texture");
                    }
                }
            } else {
                lightmap_indices.push_back(i);
                black_shadow_indices.push_back(i);
                if (any(cva->material.occluded_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK) &&
                    (l.lightmap_color == nullptr))
                {
                    THROW_OR_ABORT("Black shadow has no color texture");
                }
                if (any(cva->material.occluded_pass & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK) &&
                    (l.lightmap_depth == nullptr))
                {
                    THROW_OR_ABORT("Black shadow has no depth texture");
                }
            }
        }
        bool no_light_active = light_noshadow_indices.empty() && light_shadow_indices.empty();
        if (no_light_active) {
            filtered_lights.clear();
            black_shadow_indices.clear();
            lightmap_indices.clear();
        }
        if (cva->material.contains_skidmarks && (!no_light_active || any(emissive != 0.f))) {
            filtered_skidmarks = std::vector(skidmarks.begin(), skidmarks.end());
        }
    }
    std::vector<BlendMapTextureAndId> blended_textures_color(cva->material.textures_color.size());
    std::unordered_map<ColormapPtr, IdAndTexture> texture_ids_color;
    std::unordered_map<ColormapPtr, IdAndTexture> texture_ids_specular;
    std::unordered_map<ColormapPtr, IdAndTexture> texture_ids_normal;
    for (size_t i = 0; i < blended_textures_color.size(); ++i) {
        const auto& c = cva->material.textures_color[i];
        auto& b = blended_textures_color[i];
        if (!c.texture_descriptor.color.filename->empty()) {
            const auto& texture = secondary_rendering_resources_.contains_texture(c.texture_descriptor.color)
                ? *secondary_rendering_resources_.get_texture(c.texture_descriptor.color, TextureRole::COLOR_FROM_DB)
                : *rcva_->rendering_resources_.get_texture(c.texture_descriptor.color, TextureRole::COLOR_FROM_DB);
            auto it = texture_ids_color.try_emplace(c.texture_descriptor.color, texture_ids_color.size(), texture);
            b.id_color = it.first->second.id;
            b.tex_color = &texture;
        } else {
            b.id_color = SIZE_MAX;
            b.tex_color = nullptr;
        }
        if (!c.texture_descriptor.specular.filename->empty()) {
            const auto& texture = *rcva_->rendering_resources_.get_texture(c.texture_descriptor.specular);
            auto it = texture_ids_specular.try_emplace(c.texture_descriptor.specular, texture_ids_specular.size(), texture);
            b.id_specular = it.first->second.id;
            b.tex_specular = &texture;
        } else {
            b.id_specular = SIZE_MAX;
            b.tex_specular = nullptr;
        }
        if (!c.texture_descriptor.normal.filename->empty()) {
            const auto& texture = *rcva_->rendering_resources_.get_texture(c.texture_descriptor.normal);
            auto it = texture_ids_normal.try_emplace(c.texture_descriptor.normal, texture_ids_normal.size(), texture);
            b.id_normal = it.first->second.id;
            b.tex_normal = &texture;
        } else {
            b.id_normal = SIZE_MAX;
            b.tex_normal = nullptr;
        }
        b.ops = &c;
    }
    std::vector<BlendMapTextureAndId> blended_textures_alpha(cva->material.textures_alpha.size());
    std::unordered_map<ColormapPtr, IdAndTexture> texture_ids_alpha;
    for (size_t i = 0; i < blended_textures_alpha.size(); ++i) {
        const auto& c = cva->material.textures_alpha[i];
        auto& b = blended_textures_alpha[i];
        assert_true(!c.texture_descriptor.color.filename->empty());
        const auto& texture = secondary_rendering_resources_.contains_texture(c.texture_descriptor.color)
            ? *secondary_rendering_resources_.get_texture(c.texture_descriptor.color, TextureRole::COLOR_FROM_DB)
            : *rcva_->rendering_resources_.get_texture(c.texture_descriptor.color, TextureRole::COLOR_FROM_DB);
        auto it = texture_ids_alpha.try_emplace(c.texture_descriptor.color, texture_ids_alpha.size(), texture);
        b.id_color = it.first->second.id;
        b.id_specular = SIZE_MAX;
        b.id_normal = SIZE_MAX;
        b.tex_color = &texture;
        b.tex_specular = nullptr;
        b.tex_normal = nullptr;
        b.ops = &c;
    }
    auto check_sanity_common = [&cva](const std::vector<BlendMapTexture>& textures){
        for (const auto& t : textures) {
            if (t.texture_descriptor.color.filename->empty()) {
                THROW_OR_ABORT("Empty color or alpha texture not supported, cva: " + cva->name.full_name());
            }
            if (t.texture_descriptor.color.color_mode == ColorMode::UNDEFINED) {
                THROW_OR_ABORT("Material's color or alpha texture \"" + *t.texture_descriptor.color.filename + "\" has undefined color mode");
            }
        }
    };
    check_sanity_common(cva->material.textures_color);
    if (cva->material.textures_alpha.empty()) {
        for (const auto& [i, t] : enumerate(cva->material.textures_color)) {
            if (i == 0) {
                auto alpha_required =
                    (cva->material.blend_mode != BlendMode::OFF) &&
                    !any(cva->material.blend_mode & BlendMode::ADD_MASK);
                if (!alpha_required && !any(t.texture_descriptor.color.color_mode & ColorMode::RGB)) {
                    THROW_OR_ABORT("Opaque material's color texture \"" + *t.texture_descriptor.color.filename + "\" was not loaded as RGB");
                }
                if (alpha_required && !any(t.texture_descriptor.color.color_mode & ColorMode::RGBA)) {
                    THROW_OR_ABORT("Transparent material's color texture \"" + *t.texture_descriptor.color.filename + "\" was not loaded as RGBA");
                }
            }
        }
    } else {
        check_sanity_common(cva->material.textures_alpha);
        if (cva->material.blend_mode == BlendMode::OFF) {
            THROW_OR_ABORT("Blend-mode is off despite alpha texture, cva: " + cva->name.full_name());
        }
        for (const auto& t : cva->material.textures_color) {
            if (t.texture_descriptor.color.color_mode != ColorMode::RGB) {
                THROW_OR_ABORT("Color-texture \"" + *t.texture_descriptor.color.filename + "\" was not loaded as RGB, but an alpha-texture was set");
            }
        }
        for (const auto& t : cva->material.textures_alpha) {
            if (t.texture_descriptor.color.color_mode != ColorMode::GRAYSCALE) {
                THROW_OR_ABORT("Alpha-texture \"" + *t.texture_descriptor.color.filename + "\" was not loaded as grayscale");
            }
        }
    }
    FixedArray<float, 3> fresnel_emissive = fixed_zeros<float, 3>();
    FresnelReflectance fresnel;
    bool any_light_has_ambient = false;
    bool any_light_has_diffuse = false;
    bool any_light_has_specular = false;
    if (!filtered_lights.empty() && !is_lightmap) {
        FixedArray<float, 3> sum_light_fresnel_ambient = fixed_zeros<float, 3>();
        FixedArray<float, 3> sum_light_fog_ambient = fixed_zeros<float, 3>();
        for (const auto& [_, light] : filtered_lights) {
            if (light->emits_colors()) {
                sum_light_fresnel_ambient += light->fresnel_ambient;
                sum_light_fog_ambient += light->fog_ambient;
                any_light_has_ambient |= any((light->ambient * ambient) != 0.f);
                any_light_has_diffuse |= any((light->diffuse * diffuse) != 0.f);
                any_light_has_specular |= any((light->specular * specular) != 0.f);
            }
        }
        fresnel_emissive = sum_light_fresnel_ambient * fresnel_ambient;
        if (any_light_has_specular || any(fresnel_emissive != 0.f)) {
            fresnel = color_style && (color_style->fresnel.exponent != -1.f) ? color_style->fresnel : cva->material.shading.fresnel.reflectance;
        }
        fog_emissive = sum_light_fog_ambient * cva->material.shading.fog_ambient;
    }
    if (!any_light_has_ambient) {
        ambient = 0.f;
    }
    if (!any_light_has_diffuse) {
        diffuse = 0.f;
    }
    if (!any_light_has_specular) {
        specular = 0.f;
        specular_exponent = 0.f;
    }
    if ((fresnel.exponent != 0.f) && (std::abs(fresnel.max - fresnel.min) < 1e-12)) {
        THROW_OR_ABORT("Nonzero fresnel exponent requires nonzero fresnel range");
    }
    if ((fresnel.exponent == 0.f) && ((fresnel.max != 0.f) || (fresnel.min != 0.f))) {
        THROW_OR_ABORT("Zero fresnel exponent requires zero fresnel coefficients");
    }
    TextureIndexCalculator tic;
    if (is_lightmap || (filtered_lights.empty() && all(emissive == 0.f))) {
        if ((cva->material.blend_mode != BlendMode::OFF) && (cva->material.depth_func != DepthFunc::EQUAL))
        {
            if (!texture_ids_color.empty()) {
                assert_true(!blended_textures_color.empty());
                auto& b = blended_textures_color[0];
                assert_true(b.id_color == 0);
                assert_true(b.tex_color != nullptr);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
                blended_textures_color = { b };
                texture_ids_color = {{ b.ops->texture_descriptor.color, {0, *b.tex_color} }};
#pragma GCC diagnostic pop
            }
            texture_ids_specular.clear();
            texture_ids_normal.clear();
        } else {
            texture_ids_color.clear();
            texture_ids_specular.clear();
            texture_ids_normal.clear();
            texture_ids_alpha.clear();
            blended_textures_color.clear();
            blended_textures_alpha.clear();
        }
    }
    bool fragments_depend_on_normal =
        !is_lightmap &&
        Mlib::fragments_depend_on_normal(blended_textures_color);
    const VariableAndHash<std::string>* reflection_map = nullptr;
    FixedArray<float, 3> reflectance{ 0.f };
    if (!is_lightmap &&
        !cva->material.reflection_map->empty() &&
        (!cva->material.shading.reflectance.all_equal(0.f) ||
            any(cva->material.interior_textures.set & InteriorTextureSet::ANY_SPECULAR)))
    {
        if (color_style == nullptr) {
            THROW_OR_ABORT("cva \"" + cva->name.full_name() + "\": Material with reflection map \"" + *cva->material.reflection_map + "\" has no style");
        }
        auto it = color_style->reflection_maps.find(cva->material.reflection_map);
        if (it == color_style->reflection_maps.end()) {
            THROW_OR_ABORT(
                "cva \"" + cva->name.full_name() + "\": Could not find reflection map \""
                + *cva->material.reflection_map
                + "\" in style with keys:"
                + join(", ", color_style->reflection_maps, [](const auto& s){return *s.first;}));
        }
        if (!it->second->empty()) {
            reflection_map = &it->second;
            reflectance = cva->material.shading.reflectance * color_style->reflection_strength;
            if (any(reflectance <= 0.f) && !any(cva->material.interior_textures.set & InteriorTextureSet::ANY_SPECULAR)) {
                THROW_OR_ABORT("Reflectance is not positive, and no specular interior textures were specified");
            }
        }
    }
    if (!(cva->material.reorient_uv0 ||
          any(diffuse != 0.f) ||
          (specular_exponent != 0.f) ||
          (fresnel.exponent != 0.f) ||
          fragments_depend_on_normal ||
          (any(reflectance != 0.f) && !cva->material.reflect_only_y)))
    {
        texture_ids_normal.clear();
    }
    tic.ntextures_color = texture_ids_color.size();
    tic.ntextures_normal = texture_ids_normal.size();
    tic.ntextures_alpha = texture_ids_alpha.size();
    bool has_horizontal_detailmap = false;
    has_horizontal_detailmap |= Mlib::has_horizontal_detailmap(blended_textures_color);
    has_horizontal_detailmap |= Mlib::has_horizontal_detailmap(blended_textures_alpha);
    std::vector<size_t> lightmap_indices_color = any(cva->material.occluded_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK) ? lightmap_indices : std::vector<size_t>{};
    std::vector<size_t> lightmap_indices_depth = any(cva->material.occluded_pass & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK) ? lightmap_indices : std::vector<size_t>{};
    assert_true(lightmap_indices_color.empty() || lightmap_indices_depth.empty());
    tic.ntextures_lightmaps = lightmap_indices_color.size() + lightmap_indices_depth.size();
    tic.ntextures_filtered_skidmarks = filtered_skidmarks.size();
    if (is_lightmap || cva->material.textures_color.empty() || filtered_lights.empty() || (all(specular == 0.f) && all(reflectance == 0.f))) {
        tic.ntextures_specular = 0;
    } else {
        for (const auto& [i, t] : enumerate(cva->material.textures_color)) {
            if ((i != 0) && !t.texture_descriptor.specular.filename->empty()) {
                THROW_OR_ABORT("Only the first texture can have a specularmap");
            }
        }
        tic.ntextures_specular = !cva->material.textures_color[0].texture_descriptor.specular.filename->empty();
    }
    tic.ntextures_reflection = (size_t)(!is_lightmap && (reflection_map != nullptr) && !(*reflection_map)->empty());
    tic.ntextures_dirt = ((!cva->material.dirt_texture->empty()) && !is_lightmap && !filtered_lights.empty()) ? 2 : 0;
    InteriorTextureSet interior_texture_set;
    if (is_lightmap) {
        tic.ntextures_interior = 0;
        interior_texture_set = InteriorTextureSet::NONE;
    } else {
        tic.ntextures_interior = cva->material.interior_textures.size();
        interior_texture_set = cva->material.interior_textures.set;
        assert_true(size(interior_texture_set) == tic.ntextures_interior);
    }
    bool has_instances = (rcva_->instances_ != nullptr);
    bool has_lookat = (cva->material.transformation_mode == TransformationMode::POSITION_LOOKAT);
    bool has_yangle = (cva->material.transformation_mode == TransformationMode::POSITION_YANGLE);
    bool has_rotation_quaternion = has_instances && (cva->material.transformation_mode == TransformationMode::ALL);
    OrderableFixedArray<float, 4> alpha_distances_common = cva->material.alpha_distances;
    OrderableFixedArray<float, 2> fog_distances = cva->material.shading.fog_distances;
    if (is_lightmap) {
        alpha_distances_common = default_linear_distances;
        fog_distances = default_step_distances;
    }
    if (all(fog_emissive == 0.f) &&
        all(ambient == 0.f) &&
        all(diffuse == 0.f) &&
        all(specular == 0.f) &&
        all(emissive == 0.f) &&
        all(fresnel_emissive == 0.f))
    {
        fog_distances = default_step_distances;
    }
    bool fragments_depend_on_distance = Mlib::fragments_depend_on_distance(
        fog_distances, alpha_distances_common, blended_textures_color);
    if ((tic.ntextures_color == 0) && (tic.ntextures_dirt != 0)) {
        THROW_OR_ABORT(
            "Combination of ((ntextures_color == 0) && (ntextures_dirt != 0)) is not supported. Textures: " +
            join(" ", cva->material.textures_color, [](const auto& v) { return *v.texture_descriptor.color.filename; }));
    }
    bool reorient_normals = !cva->material.cull_faces && (any(diffuse != 0.f) || any(specular != 0.f));
    if (cva->material.cull_faces && cva->material.reorient_uv0) {
        THROW_OR_ABORT("reorient_uv0 requires disabled face culling");
    }
    bool reorient_uv0 = cva->material.reorient_uv0 && (tic.ntextures_color != 0);
    bool has_dynamic_emissive = cva->material.dynamically_lighted && !any(render_pass.external.pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK);
    LOG_INFO("RenderableColoredVertexArray::render_cva get_render_program");
    assert_true(cva->material.number_of_frames > 0);
    Hasher texture_modifiers_hash;
    texture_modifiers_hash.combine(blended_textures_color.size());
    for (const auto& t : blended_textures_color) {
        texture_modifiers_hash.combine(t.ops->modifiers_hash());
        texture_modifiers_hash.combine(t.id_color);
        texture_modifiers_hash.combine(t.id_normal);
        texture_modifiers_hash.combine(t.id_specular);
    }
    texture_modifiers_hash.combine(blended_textures_alpha.size());
    for (const auto& t : blended_textures_alpha) {
        texture_modifiers_hash.combine(t.ops->modifiers_hash());
        texture_modifiers_hash.combine(t.id_color);
        texture_modifiers_hash.combine(t.id_normal);
        texture_modifiers_hash.combine(t.id_specular);
    }
    Hasher lights_hash;
    lights_hash.combine(filtered_lights.size());
    for (const auto& [_, l] : filtered_lights) {
        lights_hash.combine(l->shading_hash());
    }
    Hasher skidmarks_hash;
    skidmarks_hash.combine(filtered_skidmarks.size());
    for (const auto& [_, s] : filtered_skidmarks) {
        skidmarks_hash.combine(s->shading_hash());
    }
    bool has_discrete_atlas_texture_layer = get_has_discrete_atlas_texture_layer(*cva);
    if (has_discrete_atlas_texture_layer) {
        if (cva->material.textures_color.size() != 1) {
            THROW_OR_ABORT("Unexpected number of color textures");
        }
        const auto& t = cva->material.textures_color[0];
        auto ttype = secondary_rendering_resources_.contains_texture(t.texture_descriptor.color)
            ? secondary_rendering_resources_.texture_type(t.texture_descriptor.color, TextureRole::COLOR_FROM_DB)
            : rcva_->rendering_resources_.texture_type(t.texture_descriptor.color, TextureRole::COLOR_FROM_DB);
        if (ttype != TextureType::TEXTURE_2D_ARRAY) {
            THROW_OR_ABORT("Unexpected texture type (expected a 2D array)");
        }
    }
    IVertexData& si = rcva_->get_vertex_array(cva, TaskLocation::FOREGROUND);
    if (si.has_discrete_triangle_texture_layers() &&
        has_discrete_atlas_texture_layer)
    {
        THROW_OR_ABORT("Detected discrete texture layer per vertex and per instance");
    }
    TextureLayerProperties texture_layer_properties = TextureLayerProperties::NONE;
    if (si.has_discrete_triangle_texture_layers()) {
        add(texture_layer_properties, (
            TextureLayerProperties::DISCRETE |
            TextureLayerProperties::VERTEX |
            TextureLayerProperties::COLOR |
            TextureLayerProperties::NORMAL));
    }
    if (has_discrete_atlas_texture_layer) {
        add(texture_layer_properties, (
            TextureLayerProperties::DISCRETE |
            TextureLayerProperties::ATLAS |
            TextureLayerProperties::COLOR |
            TextureLayerProperties::NORMAL));
    }
    if (si.has_continuous_triangle_texture_layers()) {
        add(texture_layer_properties, (
            TextureLayerProperties::CONTINUOUS |
            TextureLayerProperties::VERTEX |
            TextureLayerProperties::COLOR |
            TextureLayerProperties::NORMAL));
    }
    if ((instances != nullptr) && instances->has_continuous_texture_layer()) {
        if (any(texture_layer_properties)) {
            THROW_OR_ABORT("Detected continuous texture layers in both, vertices and instances");
        }
        add(texture_layer_properties, (
            TextureLayerProperties::CONTINUOUS |
            TextureLayerProperties::VERTEX |
            TextureLayerProperties::COLOR |
            TextureLayerProperties::NORMAL));
    }
    if ((animation_state != nullptr) && animation_state->periodic_reference_time.active()) {
        if (any(texture_layer_properties)) {
            THROW_OR_ABORT("Detected continuous texture layers in both, renderable and animation");
        }
        add(texture_layer_properties, (
            TextureLayerProperties::CONTINUOUS |
            TextureLayerProperties::UNIFORM |
            TextureLayerProperties::COLOR |
            TextureLayerProperties::NORMAL));
    }
    const ColoredRenderProgram& rp = rcva_->get_render_program(
        RenderProgramIdentifier{
            .render_pass = render_pass.external.pass,
            .skidmarks_hash = skidmarks_hash,
            .nbones = rcva_->triangles_res_->bone_indices.size(),
            .blend_mode = any(render_pass.external.pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)
                ? BlendMode::CONTINUOUS
                : cva->material.blend_mode,
            .alpha_distances = alpha_distances_common,
            .fog_distances = fog_distances,
            .fog_emissive = OrderableFixedArray{fog_emissive},
            .ntextures_color = tic.ntextures_color,
            .ntextures_normal = tic.ntextures_normal,
            .ntextures_alpha = tic.ntextures_alpha,
            .has_dynamic_emissive = has_dynamic_emissive,
            .lightmap_indices_color = lightmap_indices_color,
            .lightmap_indices_depth = lightmap_indices_depth,
            .has_specularmap = (tic.ntextures_specular != 0),
            .reflectance = OrderableFixedArray{reflectance},
            .reflect_only_y = cva->material.reflect_only_y,
            .ntextures_reflection = tic.ntextures_reflection,
            .ntextures_dirt = tic.ntextures_dirt,
            .interior_texture_set = interior_texture_set,
            .facade_inner_size = cva->material.interior_textures.facade_inner_size,
            .interior_size = cva->material.interior_textures.interior_size,
            .nuv_indices = is_lightmap ? 1 : (cva->uv1.size() + 1),
            .ncweights = is_lightmap ? 0 : cva->cweight.size(),
            .has_alpha = !cva->alpha.empty(),
            .continuous_layer_x = cva->material.continuous_layer_x,
            .continuous_layer_y = cva->material.continuous_layer_y,
            .has_horizontal_detailmap = has_horizontal_detailmap,
            .dirt_color_mode = (tic.ntextures_dirt != 0)
                ? rcva_->rendering_resources_.get_texture_descriptor(cva->material.dirt_texture).color.color_mode
                : ColorMode::UNDEFINED,
            .has_instances = has_instances,
            .has_lookat = has_lookat,
            .has_yangle = has_yangle,
            .has_rotation_quaternion = has_rotation_quaternion,
            .has_uv_offset_u = (cva->material.number_of_frames != 1),  // Texture is required in lightmap also due to alpha channel.
            .texture_layer_properties = texture_layer_properties,
            .nbillboard_ids = integral_cast<BillboardId>(cva->material.billboard_atlas_instances.size()),  // Texture is required in lightmap also due to alpha channel.
            .reorient_normals = reorient_normals,
            .reorient_uv0 = reorient_uv0,
            .emissive = OrderableFixedArray{emissive},
            .ambient = OrderableFixedArray{ambient},
            .diffuse = OrderableFixedArray{diffuse},
            .specular = OrderableFixedArray{specular},
            .specular_exponent = specular_exponent,
            .fresnel_emissive = OrderableFixedArray{fresnel_emissive},
            .fresnel = fresnel,
            .alpha = cva->material.alpha,
            .orthographic = vc.orthographic(),
            .fragments_depend_on_distance = fragments_depend_on_distance,
            .fragments_depend_on_normal = fragments_depend_on_normal,
            // Not using NAN for ordering.
            .dirtmap_offset = (tic.ntextures_dirt != 0) ? secondary_rendering_resources_.get_offset(dirtmap_name) : -1234,
            .dirtmap_discreteness = (tic.ntextures_dirt != 0) ? secondary_rendering_resources_.get_discreteness(dirtmap_name) : -1234,
            .dirt_scale = (tic.ntextures_dirt != 0) ? secondary_rendering_resources_.get_scale(dirtmap_name) : -1234,
            .texture_modifiers_hash = texture_modifiers_hash,
            .lights_hash = lights_hash},
        *cva,
        filtered_lights,
        filtered_skidmarks,
        lightmap_indices,
        light_noshadow_indices,
        light_shadow_indices,
        black_shadow_indices,
        blended_textures_color,
        blended_textures_alpha);
    LOG_INFO("RenderableColoredVertexArray::render_cva glUseProgram");
    rp.use();
    LOG_INFO("RenderableColoredVertexArray::render_cva mvp");
    CHK(glUniformMatrix4fv(rp.mvp_location, 1, GL_TRUE, mvp.casted<float>().flat_begin()));
    if (cva->material.number_of_frames != 1) {
        float uv_offset_u;
        if ((animation_state != nullptr) &&
            !animation_state->aperiodic_animation_frame.is_nan())
        {
            float duration = animation_state->aperiodic_animation_frame.duration();
            if (duration == 0.f) {
                uv_offset_u = animation_state->aperiodic_animation_frame.time();
            } else {
                float frame_index = std::floor(frame_index_from_animation_state(
                    animation_state->aperiodic_animation_frame.elapsed(),
                    duration,
                    cva->material.number_of_frames));
                uv_offset_u = frame_index / (float)cva->material.number_of_frames;
            }
        } else {
            uv_offset_u = 0;
        }
        CHK(glUniform1f(rp.uv_offset_u_location, uv_offset_u));
    }
    if ((animation_state != nullptr) && animation_state->periodic_reference_time.active()) {
        if (!any(texture_layer_properties & TextureLayerProperties::UNIFORM)) {
            THROW_OR_ABORT("Periodic reference time active, but no uniform texture layer configured");
        }
        CHK(glUniform1f(
            rp.texture_layer_location_uniform,
            animation_state->periodic_reference_time.phase01(render_pass.external.time)));
    }
    if (!cva->material.billboard_atlas_instances.empty()) {
        size_t n = cva->material.billboard_atlas_instances.size();
        auto ni = integral_cast<GLsizei>(n);
        UUVector<FixedArray<float, 3>> vertex_scale(n);
        UUVector<FixedArray<float, 2>> uv_scale(n);
        UUVector<FixedArray<float, 2>> uv_offset(n);
        std::vector<GLuint> texture_layers;
        UUVector<FixedArray<float, 4>> alpha_distances_billboards;
        if (has_discrete_atlas_texture_layer) {
            texture_layers.resize(n);
        }
        if (!vc.orthographic()) {
            alpha_distances_billboards.resize(n);
        }
        for (size_t i = 0; i < n; ++i) {
            uv_offset[i] = cva->material.billboard_atlas_instances[i].uv_offset;
            uv_scale[i] = cva->material.billboard_atlas_instances[i].uv_scale;
            vertex_scale[i] = cva->material.billboard_atlas_instances[i].vertex_scale;
            if (has_discrete_atlas_texture_layer) {
                texture_layers[i] = integral_cast<GLuint>(cva->material.billboard_atlas_instances[i].texture_layer);
            }
            if (!vc.orthographic()) {
                alpha_distances_billboards[i] = cva->material.billboard_atlas_instances[i].alpha_distances;
            }
        }
        CHK(glUniform2fv(rp.uv_offset_location, ni, (const GLfloat*)uv_offset.data()));
        CHK(glUniform2fv(rp.uv_scale_location, ni, (const GLfloat*)uv_scale.data()));
        CHK(glUniform3fv(rp.vertex_scale_location, ni, (const GLfloat*)vertex_scale.data()));
        if (has_discrete_atlas_texture_layer) {
            CHK(glUniform1uiv(rp.texture_layers_location_atlas, ni, (const GLuint*)texture_layers.data()));
        }
        if (!vc.orthographic()) {
            CHK(glUniform4fv(rp.alpha_distances_location, ni, (const GLfloat*)alpha_distances_billboards.data()));
        }
    }
    if (has_dynamic_emissive) {
        FixedArray<float, 3> dynamic_emissive = dynamic_style != nullptr
            ? dynamic_style->emissive
            : fixed_zeros<float, 3>();
        CHK(glUniform3fv(rp.dynamic_emissive_location, 1, (const GLfloat*)&dynamic_emissive));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva textures");
    for (size_t i = 0; i < tic.ntextures_color; ++i) {
        CHK(glUniform1i(rp.texture_color_locations.at(i), (GLint)tic.id_color(i)));
    }
    for (size_t i = 0; i < tic.ntextures_alpha; ++i) {
        CHK(glUniform1i(rp.texture_alpha_locations.at(i), (GLint)tic.id_alpha(i)));
    }
    assert_true(lightmap_indices_color.empty() || lightmap_indices_depth.empty());
    if (!lightmap_indices_color.empty()) {
        for (auto [i, l] : enumerate(lightmap_indices_color)) {
            CHK(glUniform1i(rp.texture_lightmap_color_locations.at(l), (GLint)tic.id_light(i)));
        }
    }
    if (!lightmap_indices_depth.empty()) {
        for (auto [i, l] : enumerate(lightmap_indices_depth)) {
            CHK(glUniform1i(rp.texture_lightmap_depth_locations.at(l), (GLint)tic.id_light(i)));
        }
    }
    for (size_t i = 0; i < tic.ntextures_normal; ++i) {
        CHK(glUniform1i(rp.texture_normalmap_locations.at(i), (GLint)tic.id_normal(i)));
    }
    for (size_t i = 0; i < tic.ntextures_filtered_skidmarks; ++i) {
        CHK(glUniform1i(rp.texture_skidmark_locations.at(i), (GLint)tic.id_skidmark(i)));
    }
    if (tic.ntextures_reflection != 0) {
        CHK(glUniform1i(rp.texture_reflection_location, (GLint)tic.id_reflection()));
    }
    if (tic.ntextures_dirt != 0) {
        CHK(glUniform1i(rp.texture_dirtmap_location, (GLint)tic.id_dirt(0)));
        CHK(glUniform1i(rp.texture_dirt_location, (GLint)tic.id_dirt(1)));
    }
    for (size_t i = 0; i < tic.ntextures_interior; ++i) {
        CHK(glUniform1i(rp.texture_interiormap_location(i), (GLint)tic.id_interior(i)));
    }
    if (tic.ntextures_specular != 0) {
        CHK(glUniform1i(rp.texture_specularmap_location, (GLint)tic.id_specular()));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva lights");
    {
        bool light_dir_required = (any(diffuse != 0.f) || any(specular != 0.f));
        if (light_dir_required || fragments_depend_on_distance || fragments_depend_on_normal || (tic.ntextures_interior != 0)) {
            // CHK(glUniform3fv(rp.light_position_location, 1, t3_from_4x4(filtered_lights.front().first).flat_begin()));
            if (light_dir_required) {
                size_t i = 0;
                for (const auto& [trafo, light] : filtered_lights) {
                    if (!any(light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                        auto mz = m.irotate(z3_from_3x3(trafo.R));
                        mz /= std::sqrt(sum(squared(mz)));
                        CHK(glUniform3fv(rp.light_dir_locations.at(i), 1, mz.flat_begin()));
                    }
                    ++i;
                }
            }
        }
    }
    {
        size_t i = 0;
        for (const auto& [_, light] : filtered_lights) {
            if (any(ambient != 0.f) &&
                any(light->ambient != 0.f) &&
                !any(light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                CHK(glUniform3fv(rp.light_ambients.at(i), 1, light->ambient.flat_begin()));
            }
            if (any(diffuse != 0.f) &&
                any(light->diffuse != 0.f) &&
                !any(light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                CHK(glUniform3fv(rp.light_diffuses.at(i), 1, light->diffuse.flat_begin()));
            }
            if (any(specular != 0.f) &&
                any(light->specular != 0.f) &&
                !any(light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                CHK(glUniform3fv(rp.light_speculars.at(i), 1, light->specular.flat_begin()));
            }
            ++i;
        }
    }
    {
        bool pred0 =
            has_lookat ||
            (any(specular != 0.f) && (specular_exponent != 0.f)) ||
            any(reflectance != 0.f) ||
            any(interior_texture_set & InteriorTextureSet::ANY_SPECULAR) ||
            (fragments_depend_on_distance && !vc.orthographic()) ||
            (fresnel.exponent != 0.f);
        bool pred1 = (fog_distances != default_step_distances);
        if (pred0 || pred1 || reorient_uv0 || (tic.ntextures_interior != 0) || reorient_normals) {
            bool ortho = vc.orthographic();
            auto miv = m.inverted() * iv;
            if ((pred0 || pred1 || reorient_uv0 || reorient_normals) && ortho) {
                auto d = z3_from_3x3(miv.R);
                d /= std::sqrt(sum(squared(d)));
                CHK(glUniform3fv(rp.view_dir, 1, d.flat_begin()));
            }
            if ((pred0 && !ortho) || (tic.ntextures_interior != 0) || pred1) {
                CHK(glUniform3fv(rp.view_pos, 1, miv.t.casted<float>().flat_begin()));
            }
        }
    }
    if (any(reflectance != 0.f) || any(interior_texture_set & InteriorTextureSet::ANY_SPECULAR)) {
        CHK(glUniformMatrix3fv(rp.r_location, 1, GL_TRUE, m.R.flat_begin()));
    }
    if (!rcva_->triangles_res_->bone_indices.empty()) {
        for (const auto& [i, l] : enumerate(absolute_bone_transformations)) {
            CHK(glUniform3fv(rp.pose_positions.at(i), 1, l.t.flat_begin()));
            CHK(glUniform4fv(rp.pose_quaternions.at(i), 1, l.q.v.flat_begin()));
        }
    }
    if (has_horizontal_detailmap) {
        if (cva->material.period_world == 0.f) {
            THROW_OR_ABORT("Horizontal detailmap requires world period");
        }
        // Moving nodes should use the uv1, uv2, ... fields for detailmaps.
        // if (render_pass.internal != InternalRenderPass::AGGREGATE) {
        //     THROW_OR_ABORT("Horizontal detailmap requires aggregation");
        // }
        FixedArray<float, 2> rem{
            (float)std::remainder(m.t(0), cva->material.period_world),
            (float)std::remainder(m.t(2), cva->material.period_world)};
        CHK(glUniform2fv(rp.horizontal_detailmap_remainder, 1, rem.flat_begin()));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind texture");
    auto setup_texture_generic = [&cva, &render_pass](
        WrapMode wrap_mode_s,
        WrapMode wrap_mode_t,
        MipmapMode mipmap_mode,
        GLenum target)
    {
        CHK(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_mode_to_native(wrap_mode_s)));
        CHK(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_mode_to_native(wrap_mode_t)));
        if (mipmap_mode != MipmapMode::NO_MIPMAPS) {
            CHK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        } else {
            CHK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        }
        if (any(render_pass.external.pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
            CHK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        } else {
            if (cva->material.magnifying_interpolation_mode == InterpolationMode::NEAREST) {
                CHK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            } else if (cva->material.magnifying_interpolation_mode == InterpolationMode::LINEAR) {
                CHK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            } else {
                THROW_OR_ABORT("Unknown interpolation mode");
            }
        }
    };
    auto setup_texture = [&setup_texture_generic](
        const ITextureHandle& h,
        TextureLayerProperties tlp)
    {
        auto cont = all(tlp, TextureLayerProperties::CONTINUOUS | TextureLayerProperties::COLOR);
            auto disc = all(tlp, TextureLayerProperties::DISCRETE | TextureLayerProperties::COLOR);
            auto mip2 = (h.mipmap_mode() == MipmapMode::WITH_MIPMAPS_2D);
        GLenum target = (cont && !mip2)
            ? GL_TEXTURE_3D
            : (disc || mip2)
                ? GL_TEXTURE_2D_ARRAY
                : GL_TEXTURE_2D;
        CHK(glBindTexture(target, h.handle<GLuint>()));
        setup_texture_generic(h.wrap_modes(0), h.wrap_modes(1), h.mipmap_mode(), target);
    };
    if (tic.ntextures_color != 0) {
        for (const auto& [c, i] : texture_ids_color) {
            LOG_INFO("RenderableColoredVertexArray::render_cva bind texture \"" + c->filename + '"');
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_color(i.id))));
            LOG_INFO("RenderableColoredVertexArray::render_cva clamp texture \"" + c->filename + '"');
            setup_texture(i.texture, texture_layer_properties);
        }
    }
    if (tic.ntextures_alpha != 0) {
        for (const auto& [c, i] : texture_ids_alpha) {
            LOG_INFO("RenderableColoredVertexArray::render_cva bind texture \"" + c->filename + '"');
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_alpha(i.id))));
            LOG_INFO("RenderableColoredVertexArray::render_cva clamp texture \"" + c->filename + '"');
            setup_texture(i.texture, TextureLayerProperties::NONE);
        }
    }
    assert_true(lightmap_indices_color.empty() || lightmap_indices_depth.empty());
    LOG_INFO("RenderableColoredVertexArray::render_cva bind light color textures");
    if (!lightmap_indices_color.empty()) {
        for (auto [i, l] : enumerate(lightmap_indices)) {
            const auto& light = *filtered_lights.at(l).second;
            if (!light.vp.has_value()) {
                THROW_OR_ABORT("Lightmap has no VP");
            }
            if (light.lightmap_color == nullptr) {
                THROW_OR_ABORT("Lightmap has no color texture");
            }
            auto mvp_light = dot2d(*light.vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(l), 1, GL_TRUE, mvp_light.casted<float>().flat_begin()));

            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_light(i))));
            CHK(glBindTexture(GL_TEXTURE_2D, light.lightmap_color->handle<GLuint>()));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
            auto border_brightness = (float)!any(filtered_lights.at(l).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK);
            float border_color[] = { border_brightness, border_brightness, border_brightness, 1.f };
            CHK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color));
            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind light depth textures");
    if (!lightmap_indices_depth.empty()) {
        for (auto [i, l] : enumerate(lightmap_indices)) {
            const auto& light = *filtered_lights.at(l).second;
            if (!light.vp.has_value()) {
                THROW_OR_ABORT("Lightmap has no VP");
            }
            if (light.lightmap_depth == nullptr) {
                THROW_OR_ABORT("Lightmap has no depth texture");
            }
            auto mvp_light = dot2d(*light.vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(l), 1, GL_TRUE, mvp_light.casted<float>().flat_begin()));

            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_light(i))));
            CHK(glBindTexture(GL_TEXTURE_2D, light.lightmap_depth->handle<GLuint>()));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind normalmap texture");
    if (tic.ntextures_normal != 0) {
        for (const auto& [c, i] : texture_ids_normal) {
            assert_true(!c->filename->empty());
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_normal(i.id))));
            setup_texture(i.texture, texture_layer_properties);
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind skidmark texture");
    for (const auto& [i, s] : enumerate(filtered_skidmarks)) {
        const auto& skidmark = *s.second;
        if (skidmark.texture == nullptr) {
            THROW_OR_ABORT("Skidmark has no texture");
        }
        auto mvp_skidmark = dot2d(skidmark.vp, m.affine());
        CHK(glUniformMatrix4fv(rp.mvp_skidmarks_locations.at(i), 1, GL_TRUE, mvp_skidmark.casted<float>().flat_begin()));

        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_skidmark(i))));
        CHK(glBindTexture(GL_TEXTURE_2D, skidmark.texture->handle<GLuint>()));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
        float borderColor[] = { 1.f, 1.f, 1.f, 1.f};
        CHK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind reflection texture");
    if (tic.ntextures_reflection != 0) {
        assert_true(reflection_map != nullptr);
        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_reflection())));
        CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, rcva_->rendering_resources_.get_texture(ColormapWithModifiers{
            .filename = *reflection_map,
            .color_mode = ColorMode::RGB,
            .mipmap_mode = MipmapMode::WITH_MIPMAPS}.compute_hash())->handle<GLuint>()));
        CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind dirtmap texture");
    if (tic.ntextures_dirt != 0) {
        const auto& mname = dirtmap_name;
        {
            const auto& dirtmap_vp = secondary_rendering_resources_.get_vp(mname);
            auto mvp_dirtmap = dot2d(dirtmap_vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_dirtmap_location, 1, GL_TRUE, mvp_dirtmap.casted<float>().flat_begin()));
        }

        {
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_dirt(0))));
            auto dname = secondary_rendering_resources_.get_alias(mname);
            const auto& texture = secondary_rendering_resources_.contains_texture_descriptor(dname)
                ? *secondary_rendering_resources_.get_texture(dname)
                : *rcva_->rendering_resources_.get_texture(dname);
            if (texture.color_mode() != ColorMode::GRAYSCALE) {
                THROW_OR_ABORT("Dirtmap \"" + *dname + "\" does not have colormode grayscale");
            }
            setup_texture(texture, TextureLayerProperties::NONE);
        }

        {
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_dirt(1))));
            const auto& texture = *rcva_->rendering_resources_.get_texture(cva->material.dirt_texture);
            if ((texture.color_mode() != ColorMode::RGB) &&
                (texture.color_mode() != ColorMode::RGBA))
            {
                THROW_OR_ABORT("Dirt texture \"" + *cva->material.dirt_texture + "\" does not have colormode RGB or RGBA");
            }
            setup_texture(texture, TextureLayerProperties::NONE);
        }
    }
    for (size_t i = 0; i < tic.ntextures_interior; ++i) {
        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_interior(i))));
        const auto& h = *rcva_->rendering_resources_.get_texture(cva->material.interior_textures[i]);
        if (h.color_mode() != ColorMode::RGB) {
            THROW_OR_ABORT("Interiormap texture \"" + *cva->material.interior_textures[i] + "\" does not have color mode RGB");
        }
        setup_texture(h, TextureLayerProperties::NONE);
    }
    if (tic.ntextures_specular != 0) {
        assert_true(tic.ntextures_specular == 1);
        assert_true(!cva->material.textures_color.empty());
        const auto& desc = cva->material.textures_color[0].texture_descriptor;
        assert_true(!desc.specular.filename->empty());
        const auto& texture = *rcva_->rendering_resources_.get_texture(desc.specular);
        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_specular())));
        setup_texture(texture, TextureLayerProperties::NONE);
    }
    if ((render_pass.external.pass != ExternalRenderPassType::DIRTMAP) &&
        !is_lightmap &&
        (cva->material.draw_distance_noperations > 0) &&
        (
            std::isnan(render_config.draw_distance_add) ||
            (render_config.draw_distance_add != INFINITY)))
    {
        if (!rcva_->triangles_res_->bone_indices.empty()) {
            THROW_OR_ABORT("Draw distance incompatible with animations");
        }
        // This is legacy code kept in case it finds a new use case.
        // As of now, deleting triangles far away is done during
        // the aggregation step, which also converts double to float,
        // making the following code obsolete.
        si.delete_triangles_far_away_legacy(
            iv.t.casted<float>(),
            m.casted<float, float>(),
            std::isnan(render_config.draw_distance_add)
                ? cva->material.draw_distance_add
                : render_config.draw_distance_add,
            std::isnan(render_config.draw_distance_slop)
                ? cva->material.draw_distance_slop
                : render_config.draw_distance_slop,
            cva->material.draw_distance_noperations,
            true,  // run_in_background
            true); // is_static
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva glBindVertexArray");
    {
        // AperiodicLagFinder lag_finder{ "draw " + cva->name + ": ", std::chrono::milliseconds{5} };
        MaterialRenderConfigGuard mrcf{ cva->material };
        if (has_instances) {
            if (any(render_pass.internal & InternalRenderPass::PRELOADED) &&
                instances->copy_in_progress())
            {
                verbose_abort("Preloaded render pass has incomplete instances: \"" + cva->name.full_name() + '"');
            }
            instances->wait();
        }
        if (any(render_pass.internal & InternalRenderPass::PRELOADED) &&
            si.copy_in_progress())
        {
            verbose_abort("Preloaded render pass has incomplete triangles: \"" + cva->name.full_name() + '"');
        }
        si.update_legacy();
        si.bind();
        LOG_INFO("RenderableColoredVertexArray::render_cva glDrawArrays");
        if (has_instances) {
            try {
                notify_rendering(CURRENT_SOURCE_LOCATION);
                TemporarilyIgnoreFloatingPointExeptions ignore_except;
                CHK(glDrawArraysInstanced(GL_TRIANGLES, 0, integral_cast<GLsizei>(3 * si.ntriangles()), instances->num_instances()));
            } catch (const std::runtime_error& e) {
                throw std::runtime_error(
                    (std::stringstream() <<
                    "Could not render instanced triangles. "
                    "#triangles: " << si.ntriangles() <<
                    ", #instances: " << instances->num_instances() <<
                    ", name: " << cva->name <<
                    ", material: " << cva->material.identifier() <<
                    ", physics material: " << physics_material_to_string(cva->morphology.physics_material) <<
                    ", " << e.what()).str());
            }
        } else {
            try {
                notify_rendering(CURRENT_SOURCE_LOCATION);
                TemporarilyIgnoreFloatingPointExeptions ignore_except;
                CHK(glDrawArrays(GL_TRIANGLES, 0, integral_cast<GLsizei>(3 * si.ntriangles())));
            } catch (const std::runtime_error& e) {
                throw std::runtime_error(
                    (std::stringstream() <<
                    "Could not render triangles. "
                    "#triangles: " << si.ntriangles() <<
                    ", name: " << cva->name <<
                    ", material: " << cva->material.identifier() <<
                    ", physics material: " << physics_material_to_string(cva->morphology.physics_material) <<
                    ", " << e.what()).str());
            }
        }
        CHK(glBindVertexArray(0));
    }
    // CHK(glFlush());
    LOG_INFO("RenderableColoredVertexArray::render_cva glDrawArrays finished");
}

void RenderableColoredVertexArray::render(
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
    LOG_FUNCTION("RenderableColoredVertexArray::render");
    if (render_pass.external.pass == ExternalRenderPassType::DIRTMAP) {
        return;
    }
    #ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
    #endif
    UUVector<OffsetAndQuaternion<float, float>> absolute_bone_transformations =
        calculate_absolute_bone_transformations(animation_state);
    for (auto& cva : aggregate_off_) {
        // if (cva->name.find("street") != std::string::npos) {
        //     continue;
        // }
        // if (cva->name.find("path") != std::string::npos) {
        //     continue;
        // }
        render_cva(
            cva,
            absolute_bone_transformations,
            mvp,
            m,
            iv,
            dynamic_style,
            lights,
            skidmarks,
            scene_graph_config,
            render_config,
            render_pass,
            animation_state,
            color_style);
    }
}

PhysicsMaterial RenderableColoredVertexArray::physics_attributes() const {
    auto result = PhysicsMaterial::NONE;
    for (const auto& m : aggregate_off_) {
        result |= m->morphology.physics_material;
    }
    for (const auto& m : aggregate_once_) {
        result |= m->morphology.physics_material;
    }
    for (const auto& m : saggregate_sorted_continuously_) {
        result |= m->morphology.physics_material;
    }
    for (const auto& m : daggregate_sorted_continuously_) {
        result |= m->morphology.physics_material;
    }
    for (const auto& m : instances_once_) {
        result |= m->morphology.physics_material;
    }
    for (const auto& m : instances_sorted_continuously_) {
        result |= m->morphology.physics_material;
    }
    for (const auto& m : sphysics_) {
        result |= m->morphology.physics_material;
    }
    for (const auto& m : dphysics_) {
        result |= m->morphology.physics_material;
    }
    return result;
}

RenderingStrategies RenderableColoredVertexArray::rendering_strategies() const {
    auto result = RenderingStrategies::NONE;
    if (!aggregate_off_.empty()) {
        result |= RenderingStrategies::OBJECT;
    }
    if (!aggregate_once_.empty()) {
        result |= RenderingStrategies::MESH_ONCE;
    }
    if (!saggregate_sorted_continuously_.empty() ||
        !daggregate_sorted_continuously_.empty())
    {
        result |= RenderingStrategies::MESH_SORTED_CONTINUOUSLY;
    }
    if (!instances_once_.empty()) {
        result |= RenderingStrategies::INSTANCES_ONCE;
    }
    if (!instances_sorted_continuously_.empty()) {
        result |= RenderingStrategies::INSTANCES_SORTED_CONTINUOUSLY;
    }
    return result;
}

bool RenderableColoredVertexArray::requires_render_pass(ExternalRenderPassType render_pass) const {
    if (aggregate_off_.empty()) {
        return false;
    }
    if (!any(physics_attributes() & PhysicsMaterial::ATTR_VISIBLE)) {
        return false;
    }
    if (any(render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        return required_occluder_passes_.contains(render_pass);
    }
    return true;
}

bool RenderableColoredVertexArray::requires_blending_pass(ExternalRenderPassType render_pass) const {
    if (!requires_blending_pass_) {
        return false;
    }
    if (any(render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        return required_occluder_passes_.contains(render_pass);
    }
    return true;
}

int RenderableColoredVertexArray::continuous_blending_z_order() const {
    if (continuous_blending_z_order_ == CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING) {
        THROW_OR_ABORT("Conflicting z_orders");
    }
    return continuous_blending_z_order_;
}

void RenderableColoredVertexArray::append_physics_to_queue(
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& float_queue,
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& double_queue) const
{
    for (const auto& e : sphysics_) {
        float_queue.push_back(e);
    }
    for (const auto& e : dphysics_) {
        double_queue.push_back(e);
    }
}

void RenderableColoredVertexArray::append_sorted_aggregates_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const
{
    for (const auto& cva : saggregate_sorted_continuously_) {
        VisibilityCheck vc{mvp};
        if (vc.is_visible(cva->name.full_name(), cva->material, cva->morphology, BILLBOARD_ID_NONE, scene_graph_config, external_render_pass.pass))
        {
            TransformationMatrix<float, float, 3> mo{m.R, (m.t - offset).casted<float>()};
            aggregate_queue.push_back({ (float)vc.sorting_key(cva->material), cva->transformed<float>(mo, "_transformed_tm") });
        }
    }
    for (const auto& cva : daggregate_sorted_continuously_) {
        VisibilityCheck vc{mvp};
        if (vc.is_visible(cva->name.full_name(), cva->material, cva->morphology, BILLBOARD_ID_NONE, scene_graph_config, external_render_pass.pass))
        {
            TransformationMatrix<float, ScenePos, 3> mo{m.R, m.t - offset};
            aggregate_queue.push_back({ (float)vc.sorting_key(cva->material), cva->transformed<float>(mo, "_transformed_tm") });
        }
    }
}

void RenderableColoredVertexArray::append_large_aggregates_to_queue(
    const TransformationMatrix<float, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const
{
    for (const auto& cva : aggregate_once_) {
        TransformationMatrix<float, ScenePos, 3> mo{m.R, m.t - offset};
        aggregate_queue.push_back(cva->transformed<float>(mo, "_transformed_tm"));
    }
}

void RenderableColoredVertexArray::append_sorted_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    SmallInstancesQueues& instances_queues) const
{
    instances_queues.insert(
        instances_sorted_continuously_,
        mvp,
        m,
        offset,
        billboard_id,
        scene_graph_config);
}

void RenderableColoredVertexArray::append_large_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    LargeInstancesQueue& instances_queue) const
{
    instances_queue.insert(
        instances_once_,
        mvp,
        m,
        offset,
        billboard_id,
        scene_graph_config,
        InvisibilityHandling::RAISE);
    if (any(instances_queue.render_pass() & ExternalRenderPassType::IS_GLOBAL_MASK)) {
        instances_queue.insert(
            instances_sorted_continuously_,
            mvp,
            m,
            offset,
            billboard_id,
            scene_graph_config,
            InvisibilityHandling::SKIP);
    }
}

void RenderableColoredVertexArray::extend_aabb(
    const TransformationMatrix<float, ScenePos, 3>& mv,
    ExternalRenderPassType render_pass,
    AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const
{
    auto extend = [&](auto& cvas){
        for (const auto& cva : cvas) {
            if (!any(cva->material.occluded_pass & render_pass)) {
                continue;
            }
            for (const auto& t : cva->triangles) {
                for (const auto& v : t.flat_iterable()) {
                    aabb.extend(
                        mv.transform(funpack(v.position))
                        .template casted<CompressedScenePos>());
                }
            }
        }
    };
    extend(aggregate_off_);
    extend(aggregate_once_);
    extend(saggregate_sorted_continuously_);
    extend(daggregate_sorted_continuously_);
    extend(instances_once_);
    extend(instances_sorted_continuously_);
}

ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3> RenderableColoredVertexArray::aabb() const {
    return aabb_;
}

ExtremalBoundingSphere<CompressedScenePos, 3> RenderableColoredVertexArray::bounding_sphere() const {
    return bounding_sphere_;
}

ScenePos RenderableColoredVertexArray::max_center_distance2(BillboardId billboard_id) const {
    ScenePos result = 0.;
    for (const auto& cva : aggregate_off_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : aggregate_once_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : saggregate_sorted_continuously_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : daggregate_sorted_continuously_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : instances_once_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : instances_sorted_continuously_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    // if (result == 0.) {
    //     THROW_OR_ABORT("Could not calculate visibility AABB, renderable seems to be empty");
    // }
    return result;
}

void RenderableColoredVertexArray::print_stats(std::ostream& ostr) const {
    auto print_list = [&ostr]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas, const std::string& name) {
        ostr << name << '\n';
        ostr << "#triangle lists: " << cvas.size() << '\n';
        size_t i = 0;
        for (auto& cva : cvas) {
            ostr << "triangle list " << i << " #lines: " << cva->lines.size() << '\n';
            ostr << "triangle list " << i << " #tris:  " << cva->triangles.size() << '\n';
        }
    };
    print_list(aggregate_off_, "aggregate_off");
    print_list(aggregate_once_, "aggregate_once");
    print_list(saggregate_sorted_continuously_, "saggregate_sorted_continuously");
    print_list(daggregate_sorted_continuously_, "daggregate_sorted_continuously");
    print_list(instances_once_, "instances_once");
    print_list(instances_sorted_continuously_, "instances_sorted_continuously");
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RenderableColoredVertexArray& rcvi)
{
    rcvi.print_stats(ostr);
    return ostr;
}
