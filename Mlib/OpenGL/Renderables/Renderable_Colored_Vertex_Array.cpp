#include "Renderable_Colored_Vertex_Array.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Primitives/Frustum3.hpp>
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene_Config/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Instances/Large_Instances_Queue.hpp>
#include <Mlib/Scene_Graph/Instances/Small_Instances_Queues.hpp>
#include <Mlib/Scene_Graph/Render/Batch_Renderers/Task_Location.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Object_Factory.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <climits>
#include <stdexcept>
#include <unordered_map>
#ifndef WITHOUT_GRAPHICS
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Toggle_Benchmark_Rendering.hpp>
#include <Mlib/OpenGL/Frame_Index_From_Animation_Time.hpp>
#include <Mlib/OpenGL/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/OpenGL/Instance_Handles/Wrap_Mode.hpp>
#endif

// #undef LOG_FUNCTION
// #undef LOG_INFO
// #define LOG_FUNCTION(msg) ::Mlib::Log log(msg)
// #define LOG_INFO(msg) log.info(msg)
// #define LOG_INFO(msg) linfo() << msg

using namespace Mlib;

#ifndef WITHOUT_GRAPHICS
static const int CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED = INT_MAX;
static const int CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING = INT_MIN;
#endif

inline bool has_instances(const IGpuVertexData& v) {
    return false;
}

inline bool has_instances(const IGpuVertexArray& v) {
    return (v.instances() != nullptr);
}

template <class T>
inline bool has_instances(const ColoredVertexArray<T>& v) {
    return false;
}

inline std::shared_ptr<IGpuVertexData> RenderableColoredVertexArray::to_gpu_data(
    std::shared_ptr<IGpuVertexData> v,
    CachingBehavior) const
{
    return v;
}

inline std::shared_ptr<IGpuVertexData> RenderableColoredVertexArray::to_gpu_data(
    std::shared_ptr<ColoredVertexArray<float>> v,
    CachingBehavior caching_behavior) const
{
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::to_gpu_data despite no graphics");
    #else
    return rcva_->gpu_object_factory_.create_vertex_data(v, rcva_->triangles_res_, caching_behavior, TaskLocation::BACKGROUND);
    #endif
}

inline std::shared_ptr<IGpuVertexArray> RenderableColoredVertexArray::to_gpu_array(
    std::shared_ptr<IGpuVertexArray> v,
    CachingBehavior) const
{
    return v;
}

inline std::shared_ptr<IGpuVertexArray> RenderableColoredVertexArray::to_gpu_array(
    std::shared_ptr<ColoredVertexArray<float>> v,
    CachingBehavior caching_behavior) const
{
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::to_gpu_data despite no graphics");
    #else
    return rcva_->gpu_object_factory_.create_vertex_array(v, rcva_->triangles_res_, caching_behavior, TaskLocation::BACKGROUND);
    #endif
}

inline const MeshMeta& get_meta(const std::shared_ptr<IGpuVertexData>& v) {
    return v->mesh_meta();
}

inline const MeshMeta& get_meta(const std::shared_ptr<IGpuVertexArray>& v) {
    return v->vertices()->mesh_meta();
}

template <class T>
inline const MeshMeta& get_meta(const std::shared_ptr<ColoredVertexArray<T>>& v) {
    return v->meta;
}

template <class T>
struct FloatingPointType {};

template<>
struct FloatingPointType<IGpuVertexData> {
    using value_type = float;
};

template<>
struct FloatingPointType<IGpuVertexArray> {
    using value_type = float;
};

template<>
struct FloatingPointType<ColoredVertexArray<float>> {
    using value_type = float;
};

template<>
struct FloatingPointType<ColoredVertexArray<CompressedScenePos>> {
    using value_type = CompressedScenePos;
};

RenderableColoredVertexArray::RenderableColoredVertexArray(
    #ifndef WITHOUT_GRAPHICS
    RenderingResources& rendering_resources,
    #endif
    const std::shared_ptr<const ColoredVertexArrayResource>& rcva,
    CachingBehavior caching_behavior,
    const RenderableResourceFilter& renderable_resource_filter)
    : rcva_{ rcva }
    #ifndef WITHOUT_GRAPHICS
    , continuous_blending_z_order_{ CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED }
    , gpu_vertex_array_renderer_{ RenderingContextStack::primary_rendering_resources(), rendering_resources }
    , required_blending_passes_{ BlendingPassType::NONE }
    , aabb_{ ExtremalBoundingVolume::EMPTY }
    , bounding_sphere_{ ExtremalBoundingVolume::EMPTY }
    #endif
{
    #ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
    #endif
    auto add_cvas = [&]<typename TArray>(const std::list<std::shared_ptr<TArray>>& cvas)
    {
        using TPos = FloatingPointType<TArray>::value_type;
        for (const auto& [i, t] : enumerate(cvas)) {
            const auto& meta = get_meta(t);
            if (renderable_resource_filter.matches(i, meta)) {
                #ifndef WITHOUT_GRAPHICS
                if (any(meta.morphology.physics_material & PhysicsMaterial::ATTR_VISIBLE)) {
                    if ((meta.material.aggregate_mode == AggregateMode::NONE) ||
                        has_instances(*t))
                    {
                        if constexpr (!std::is_same_v<TArray, IGpuVertexData> && std::is_same_v<TPos, float>) {
                            aggregate_off_.push_back(to_gpu_array(t, caching_behavior));
                            required_occluder_passes_.insert(meta.material.occluder_pass);
                        } else {
                            throw std::runtime_error("Instances and aggregate=off require single precision (material: " + meta.material.identifier() + ')');
                        }
                    } else if (meta.material.aggregate_mode == AggregateMode::ONCE) {
                        if constexpr (std::is_same_v<TPos, CompressedScenePos>) {
                            aggregate_once_.push_back(t);
                        } else {
                            throw std::runtime_error("Aggregate=once requires double precision (material: " + meta.material.identifier() + ')');
                        }
                    } else if (meta.material.aggregate_mode == AggregateMode::SORTED_CONTINUOUSLY) {
                        if constexpr (std::is_same_v<TArray, IGpuVertexData> || std::is_same_v<TArray, IGpuVertexArray>) {
                            throw std::runtime_error("Aggregate=sorted_continuously requires arrays");
                        } else if constexpr (std::is_same_v<TPos, float>) {
                            saggregate_sorted_continuously_.push_back(t);
                        } else {
                            daggregate_sorted_continuously_.push_back(t);
                        }
                    } else if (meta.material.aggregate_mode == AggregateMode::INSTANCES_ONCE) {
                        if constexpr (!std::is_same_v<TArray, IGpuVertexArray> && std::is_same_v<TPos, float>) {
                            instances_once_.push_back(to_gpu_data(t, caching_behavior));
                        } else {
                            throw std::runtime_error("Aggregate=instances_once requires single precision (material: " + meta.material.identifier() + ')');
                        }
                    } else if (meta.material.aggregate_mode == AggregateMode::INSTANCES_SORTED_CONTINUOUSLY) {
                        if constexpr (!std::is_same_v<TArray, IGpuVertexArray> && std::is_same_v<TPos, float>) {
                            instances_sorted_continuously_.push_back(to_gpu_data(t, caching_behavior));
                        } else {
                            throw std::runtime_error("Aggregate=instances_sorted_continuously requires single precision (material: " + meta.material.identifier() + ')');
                        }
                    } else if (
                        (meta.material.aggregate_mode != AggregateMode::NODE_OBJECT) &&
                        (meta.material.aggregate_mode != AggregateMode::NODE_TRIANGLES))
                    {
                        throw std::runtime_error("Unknown aggregate mode: \"" + meta.name.full_name() + '"');
                    }
                    if ((meta.material.continuous_blending_z_order == CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED) ||
                        (meta.material.continuous_blending_z_order == CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING))
                    {
                        throw std::runtime_error("Unsupported \"continuous_blending_z_order\" value: \"" + meta.name.full_name() + '"');
                    }
                    if (continuous_blending_z_order_ != CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING) {
                        if (any(meta.material.blend_mode & BlendMode::ANY_CONTINUOUS) &&
                            (meta.material.aggregate_mode == AggregateMode::NONE))
                        {
                            if (!any(meta.material.blending_pass)) {
                                throw std::runtime_error("Blended material has no blending pass: \"" + meta.name.full_name() + '"');
                            }
                            required_blending_passes_ |= meta.material.blending_pass;
                            if (continuous_blending_z_order_ == CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED) {
                                continuous_blending_z_order_ = meta.material.continuous_blending_z_order;
                            } else if (continuous_blending_z_order_ != meta.material.continuous_blending_z_order) {
                                continuous_blending_z_order_ = CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING;
                            }
                        }
                    }
                }
                #endif
                if (any(meta.morphology.physics_material & PhysicsMaterial::ATTR_COLLIDE)) {
                    if constexpr (std::is_same_v<TArray, IGpuVertexData> || std::is_same_v<TArray, IGpuVertexArray>) {
                        throw std::runtime_error("attr_collide requires arrays: \"" + meta.name.full_name() + '"');
                    } else if constexpr (std::is_same_v<TPos, float>) {
                        sphysics_.push_back(t);
                    } else if constexpr (std::is_same_v<TPos, CompressedScenePos>) {
                        dphysics_.push_back(t);
                    } else {
                        throw std::runtime_error("Unknown physics precision: \"" + meta.name.full_name() + '"');
                    }
                }
            }
        }
    };
    if (rcva->triangles_res_ != nullptr) {
        if (rcva->triangles_res_->scvas.empty() &&
            rcva->triangles_res_->dcvas.empty())
        {
            throw std::runtime_error("RenderableColoredVertexArray received no arrays");
        }
        add_cvas(rcva->triangles_res_->scvas);
        add_cvas(rcva->triangles_res_->dcvas);
    }
    #ifndef WITHOUT_GRAPHICS
    add_cvas(rcva->gpu_vertex_data_);
    add_cvas(rcva->gpu_vertex_arrays_);
    #endif
    if (
        #ifndef WITHOUT_GRAPHICS
        aggregate_off_.empty() &&
        aggregate_once_.empty() &&
        saggregate_sorted_continuously_.empty() &&
        daggregate_sorted_continuously_.empty() &&
        instances_once_.empty() &&
        instances_sorted_continuously_.empty() &&
        #endif
        sphysics_.empty() &&
        dphysics_.empty())
    {
        throw std::runtime_error(
            "Filter did not match a single array.\n" +
            (std::stringstream() << renderable_resource_filter).str());
    }

    #ifndef WITHOUT_GRAPHICS
    for (auto& cva : aggregate_off_) {
        cva->extend_aabb(aabb_);
        cva->extend_bounding_sphere(bounding_sphere_);
    }
    for (auto& cva : aggregate_once_) {
        cva->extend_aabb(aabb_);
        cva->extend_bounding_sphere(bounding_sphere_);
    }
    for (auto& cva : saggregate_sorted_continuously_) {
        cva->extend_aabb(aabb_);
        cva->extend_bounding_sphere(bounding_sphere_);
    }
    for (auto& cva : daggregate_sorted_continuously_) {
        cva->extend_aabb(aabb_);
        cva->extend_bounding_sphere(bounding_sphere_);
    }
    #endif
}

RenderableColoredVertexArray::~RenderableColoredVertexArray() = default;

UUVector<OffsetAndQuaternion<float, float>> RenderableColoredVertexArray::calculate_absolute_bone_transformations(const AnimationState* animation_state) const
{
    if ((rcva_->triangles_res_ == nullptr) || rcva_->triangles_res_->bone_indices.empty()) {
        return {};
    }
    // TIME_GUARD_DECLARE(time_guard, "calculate_absolute_bone_transformations", "calculate_absolute_bone_transformations");
    if (animation_state == nullptr) {
        throw std::runtime_error("Animation without animation state");
    }
    auto get_abt = [this](const VariableAndHash<std::string>& animation_name, float time) {
        if (animation_name->empty()) {
            throw std::runtime_error("Animation frame has no name");
        }
        if (std::isnan(time)) {
            throw std::runtime_error("Vertex array loop time is NAN");
        }
        auto poses = rcva_->scene_node_resources_.get_relative_poses(
            animation_name,
            time);
        UUVector<OffsetAndQuaternion<float, float>> ms = rcva_->triangles_res_->vectorize_joint_poses(poses);
        UUVector<OffsetAndQuaternion<float, float>> absolute_bone_transformations = rcva_->triangles_res_->skeleton->rebase_to_initial_absolute_transform(ms);
        if (absolute_bone_transformations.size() != rcva_->triangles_res_->bone_indices.size()) {
            throw std::runtime_error("Number of bone indices differs from number of quaternions");
        }
        return absolute_bone_transformations;
    };
    if (animation_state->aperiodic_animation_frame.active()) {
        return get_abt(animation_state->aperiodic_skelletal_animation_name, animation_state->aperiodic_animation_frame.time());
    } else {
        return get_abt(animation_state->periodic_skelletal_animation_name, animation_state->periodic_skelletal_animation_frame.time());
    }
}

void RenderableColoredVertexArray::render(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<SceneDir, ScenePos, 3>& m,
    const TransformationMatrix<SceneDir, ScenePos, 3>& iv,
    const DynamicStyle* dynamic_style,
    const std::list<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const AnimationState* animation_state,
    const ColorStyle* color_style) const
{
    LOG_FUNCTION("RenderableColoredVertexArray::render");
    if (render_pass.rsd.external_render_pass.pass == ExternalRenderPassType::DIRTMAP) {
        return;
    }
    #ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
    #endif
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::render called without graphics support");
    #else
    UUVector<OffsetAndQuaternion<float, float>> absolute_bone_transformations =
        calculate_absolute_bone_transformations(animation_state);
    for (auto& cva : aggregate_off_) {
        // if (meta.name.find("street") != std::string::npos) {
        //     continue;
        // }
        // if (meta.name.find("path") != std::string::npos) {
        //     continue;
        // }
        gpu_vertex_array_renderer_.render(
            cva,
            rcva_->triangles_res_,
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
    #endif
}

PhysicsMaterial RenderableColoredVertexArray::physics_attributes() const {
    auto result = PhysicsMaterial::NONE;
    #ifndef WITHOUT_GRAPHICS
    for (const auto& m : aggregate_off_) {
        result |= m->vertices()->mesh_meta().morphology.physics_material;
    }
    for (const auto& m : aggregate_once_) {
        result |= m->meta.morphology.physics_material;
    }
    for (const auto& m : saggregate_sorted_continuously_) {
        result |= m->meta.morphology.physics_material;
    }
    for (const auto& m : daggregate_sorted_continuously_) {
        result |= m->meta.morphology.physics_material;
    }
    for (const auto& m : instances_once_) {
        result |= m->mesh_meta().morphology.physics_material;
    }
    for (const auto& m : instances_sorted_continuously_) {
        result |= m->mesh_meta().morphology.physics_material;
    }
    #endif
    for (const auto& m : sphysics_) {
        result |= m->meta.morphology.physics_material;
    }
    for (const auto& m : dphysics_) {
        result |= m->meta.morphology.physics_material;
    }
    return result;
}

RenderingStrategies RenderableColoredVertexArray::rendering_strategies() const {
    auto result = RenderingStrategies::NONE;
    #ifndef WITHOUT_GRAPHICS
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
    #endif
    return result;
}

bool RenderableColoredVertexArray::requires_render_pass(ExternalRenderPassType render_pass) const {
    #ifdef WITHOUT_GRAPHICS
    return false;
    #else
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
    #endif
}

BlendingPassType RenderableColoredVertexArray::required_blending_passes(ExternalRenderPassType render_pass) const {
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::required_blending_passes called without graphics support");
    #else
    if (!any(required_blending_passes_)) {
        return BlendingPassType::NONE;
    }
    if (any(render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK) &&
        !required_occluder_passes_.contains(render_pass))
    {
        return BlendingPassType::NONE;
    }
    return required_blending_passes_;
    #endif
}

int RenderableColoredVertexArray::continuous_blending_z_order() const {
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::continuous_blending_z_order called without graphics support");
    #else
    if (continuous_blending_z_order_ == CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED) {
        throw std::runtime_error("Undefined z order");
    }
    if (continuous_blending_z_order_ == CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING) {
        throw std::runtime_error("Conflicting z orders");
    }
    return continuous_blending_z_order_;
    #endif
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
    const TransformationMatrix<SceneDir, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const
{
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::append_sorted_aggregates_to_queue called without graphics support");
    #else
    for (const auto& cva : saggregate_sorted_continuously_) {
        VisibilityCheck vc{mvp};
        if (vc.is_visible(cva->meta.name.full_name_and_hash(), cva->meta.material, cva->meta.morphology, BILLBOARD_ID_NONE, scene_graph_config, external_render_pass.pass))
        {
            TransformationMatrix<float, float, 3> mo{m.R, (m.t - offset).casted<float>()};
            aggregate_queue.push_back({ (float)vc.sorting_key(cva->meta.material), cva->transformed<float>(mo, "_transformed_tm") });
        }
    }
    for (const auto& cva : daggregate_sorted_continuously_) {
        VisibilityCheck vc{mvp};
        if (vc.is_visible(cva->meta.name.full_name_and_hash(), cva->meta.material, cva->meta.morphology, BILLBOARD_ID_NONE, scene_graph_config, external_render_pass.pass))
        {
            TransformationMatrix<SceneDir, ScenePos, 3> mo{m.R, m.t - offset};
            aggregate_queue.push_back({ (float)vc.sorting_key(cva->meta.material), cva->transformed<float>(mo, "_transformed_tm") });
        }
    }
    #endif
}

void RenderableColoredVertexArray::append_large_aggregates_to_queue(
    const TransformationMatrix<SceneDir, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const
{
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::append_large_aggregates_to_queue called without graphics support");
    #else
    for (const auto& cva : aggregate_once_) {
        TransformationMatrix<SceneDir, ScenePos, 3> mo{m.R, m.t - offset};
        aggregate_queue.push_back(cva->transformed<float>(mo, "_transformed_tm"));
    }
    #endif
}

void RenderableColoredVertexArray::append_sorted_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<SceneDir, ScenePos, 3>& m,
    const TransformationMatrix<SceneDir, ScenePos, 3>& iv,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    SmallInstancesQueues& instances_queues) const
{
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::append_sorted_instances_to_queue called without graphics support");
    #else
    instances_queues.insert(
        instances_sorted_continuously_,
        mvp,
        m,
        offset,
        billboard_id,
        scene_graph_config);
    #endif
}

void RenderableColoredVertexArray::append_large_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<SceneDir, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    LargeInstancesQueue& instances_queue) const
{
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::append_large_instances_to_queue called without graphics support");
    #else
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
    #endif
}

void RenderableColoredVertexArray::extend_aabb(
    const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
    ExternalRenderPassType render_pass,
    AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const
{
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::extend_aabb called without graphics support");
    #else
    auto extend = [&](auto& cvas){
        for (const auto& cva : cvas) {
            const auto& meta = get_meta(cva);
            if (!any(meta.material.occluded_pass & render_pass)) {
                continue;
            }
            cva->extend_aabb(mv, aabb);
        }
    };
    extend(aggregate_off_);
    extend(aggregate_once_);
    extend(saggregate_sorted_continuously_);
    extend(daggregate_sorted_continuously_);
    extend(instances_once_);
    extend(instances_sorted_continuously_);
    #endif
}

ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3> RenderableColoredVertexArray::aabb() const {
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::aabb called without graphics support");
    #else
    return aabb_;
    #endif
}

ExtremalBoundingSphere<CompressedScenePos, 3> RenderableColoredVertexArray::bounding_sphere() const {
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::bounding_sphere called without graphics support");
    #else
    return bounding_sphere_;
    #endif
}

ScenePos RenderableColoredVertexArray::max_center_distance2(BillboardId billboard_id) const {
    #ifdef WITHOUT_GRAPHICS
    throw std::runtime_error("RenderableColoredVertexArray::max_center_distance2 called without graphics support");
    #else
    ScenePos result = 0.;
    for (const auto& cva : aggregate_off_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : aggregate_once_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : saggregate_sorted_continuously_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : daggregate_sorted_continuously_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : instances_once_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    for (const auto& cva : instances_sorted_continuously_) { result = std::max(result, cva->max_center_distance2(billboard_id)); }
    // if (result == 0.) {
    //     throw std::runtime_error("Could not calculate visibility AABB, renderable seems to be empty");
    // }
    return result;
    #endif
}

void RenderableColoredVertexArray::print_stats(std::ostream& ostr) const {
    auto print_list = [&ostr]<typename TArray>(const std::list<std::shared_ptr<TArray>>& cvas, const std::string& name) {
        ostr << name << '\n';
        ostr << "#triangle lists: " << cvas.size() << '\n';
        for (const auto& [i, cva] : enumerate(cvas)) {
            ostr << i << '\n';
            cva->print_stats(ostr);
        }
    };
    #ifndef WITHOUT_GRAPHICS
    print_list(aggregate_off_, "aggregate_off");
    print_list(aggregate_once_, "aggregate_once");
    print_list(saggregate_sorted_continuously_, "saggregate_sorted_continuously");
    print_list(daggregate_sorted_continuously_, "daggregate_sorted_continuously");
    print_list(instances_once_, "instances_once");
    print_list(instances_sorted_continuously_, "instances_sorted_continuously");
    #endif
    print_list(sphysics_, "sphysics_");
    print_list(dphysics_, "dphysics_");
}

#ifndef WITHOUT_GRAPHICS
void RenderableColoredVertexArray::initialize_gpu_arrays() {
    for (auto& cva : aggregate_off_) {
        if (!cva->initialized()) {
            cva->initialize();
        }
    }
}

bool RenderableColoredVertexArray::copy_in_progress() const {
    for (auto& cva : aggregate_off_) {
        if (cva->copy_in_progress()) {
            return true;
        }
    }
    return false;
}

void RenderableColoredVertexArray::wait() const {
    for (auto& cva : aggregate_off_) {
        cva->wait();
    }
}
#endif

std::ostream& Mlib::operator << (std::ostream& ostr, const RenderableColoredVertexArray& rcvi)
{
    rcvi.print_stats(ostr);
    return ostr;
}
