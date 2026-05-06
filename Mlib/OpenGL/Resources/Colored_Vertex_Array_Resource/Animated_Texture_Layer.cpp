#include "Animated_Texture_Layer.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Primitives/Extremal_Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Extremal_Bounding_Sphere.hpp>
#include <Mlib/Geometry/Primitives/Extremal_Bounding_Volume.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Trail_Sequence.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <mutex>

using namespace Mlib;

AnimatedTextureLayerBuffers::AnimatedTextureLayerBuffers(
    size_t max_num_triangles,
    const MeshMeta& mesh_meta)
    : mesh_meta_{ mesh_meta }
    , max_num_triangles_{ max_num_triangles }
    , tmp_num_triangles_{ 0 }
    , gl_num_triangles_{ 0 }
    , animation_times_(max_num_triangles)
    , animation_sequences_(max_num_triangles)
    , triangle_{ max_num_triangles }
    , texture_layer_{ max_num_triangles }
{
}

void AnimatedTextureLayerBuffers::update(std::chrono::steady_clock::time_point time) {
    if (time == std::chrono::steady_clock::time_point()) {
        throw std::runtime_error("AnimatedTextureLayer::update received uninitialized time");
    }
    if (time_ == std::chrono::steady_clock::time_point()) {
        if (tmp_num_triangles_ != 0) {
            throw std::runtime_error("AnimatedTextureLayer::update without previous move");
        }
    } else {
        auto time_offset = std::chrono::duration<float>(time - time_).count() * seconds;
        for (size_t i = 0; i < tmp_length(); ++i) {
            const auto& ai = animation_times_[i];
            const auto& bi = *animation_sequences_[i];
            texture_layer_[i] = ai.applied([&](const auto& v) { return bi.times_to_w(v + time_offset); });
        }
    }
    gl_num_triangles_ = integral_cast<GLsizei>(tmp_num_triangles_);
}

size_t AnimatedTextureLayerBuffers::ntriangles() const {
    return integral_cast<size_t>(gl_num_triangles_);
}

size_t AnimatedTextureLayerBuffers::nuvs() const {
    return 1;
}

size_t AnimatedTextureLayerBuffers::ncweights() const {
    return 0;
}

bool AnimatedTextureLayerBuffers::has_alpha() const {
    return false;
}

bool AnimatedTextureLayerBuffers::has_continuous_triangle_texture_layers() const {
    return true;
}

bool AnimatedTextureLayerBuffers::has_discrete_triangle_texture_layers() const {
    return false;
}

bool AnimatedTextureLayerBuffers::has_interiormap() const {
    return false;
}

bool AnimatedTextureLayerBuffers::has_bone_indices() const {
    return false;
}

IArrayBuffer& AnimatedTextureLayerBuffers::vertex_buffer() {
    return triangle_;
}

IArrayBuffer& AnimatedTextureLayerBuffers::bone_weight_buffer() {
    throw std::runtime_error("AnimatedTextureLayerBuffers has no bone_weight_buffer");
}

IArrayBuffer& AnimatedTextureLayerBuffers::texture_layer_buffer() {
    return texture_layer_;
}

IArrayBuffer& AnimatedTextureLayerBuffers::interior_mapping_buffer() {
    throw std::runtime_error("AnimatedTextureLayerBuffers has no interior_mapping_buffer");
}

IArrayBuffer& AnimatedTextureLayerBuffers::uv1_buffer(size_t i) {
    throw std::runtime_error("AnimatedTextureLayerBuffers has no uv1_buffer");
}

IArrayBuffer& AnimatedTextureLayerBuffers::cweight_buffer(size_t i) {
    throw std::runtime_error("AnimatedTextureLayerBuffers has no cweight_buffer");
}

IArrayBuffer& AnimatedTextureLayerBuffers::alpha_buffer() {
    throw std::runtime_error("AnimatedTextureLayerBuffers has no alpha_buffer");
}

void AnimatedTextureLayerBuffers::append(
    const FixedArray<ColoredVertex<float>, 3>& triangle,
    const FixedArray<float, 3>& time,
    const TrailSequence& sequence)
{
    if (tmp_num_triangles_ >= max_num_triangles_) {
        throw std::runtime_error("Maximum number of triangles exceeded");
    }
    triangle_.append(triangle);
    texture_layer_.append(fixed_nans<float, 3>());
    animation_times_[tmp_num_triangles_] = time;
    animation_sequences_[tmp_num_triangles_] = &sequence;
    ++tmp_num_triangles_;
}

void AnimatedTextureLayerBuffers::move(float dt, const StaticWorld& world) {
    for (size_t i = 0; i < tmp_length();) {
        auto& ai = animation_times_[i];
        auto& bi = animation_sequences_[i];
        ai += fixed_full<float, 3>(dt);
        if (any(ai <= bi->times_to_w.xmax())) {  // Note that this includes negative times, which is intended.
            ++i;
        } else {
            triangle_.remove(i);
            texture_layer_.remove(i);
            --tmp_num_triangles_;
            if (tmp_num_triangles_ != 0) {
                ai = animation_times_[tmp_num_triangles_];
                bi = animation_sequences_[tmp_num_triangles_];
            }
        }
    }
    time_ = world.time;
}

std::chrono::steady_clock::time_point AnimatedTextureLayerBuffers::time() const {
    return time_;
}

void AnimatedTextureLayerBuffers::delete_triangles_far_away_legacy(
    const FixedArray<float, 3>& position,
    const TransformationMatrix<float, float, 3>& m,
    float draw_distance_add,
    float draw_distance_slop,
    size_t noperations,
    bool run_in_background,
    bool is_static)
{
    throw std::runtime_error("AnimatedTextureLayerBuffers::delete_triangles_far_away not implemented");
}

const MeshMeta& AnimatedTextureLayerBuffers::mesh_meta() const {
    return mesh_meta_;
}

const ExtremalAxisAlignedBoundingBox<float, 3>& AnimatedTextureLayerBuffers::aabb() const {
    static auto result = ExtremalAxisAlignedBoundingBox<float, 3>{ExtremalBoundingVolume::FULL};
    return result;
}

const ExtremalBoundingSphere<float, 3>& AnimatedTextureLayerBuffers::bounding_sphere() const {
    static auto result = ExtremalBoundingSphere<float, 3>{ExtremalBoundingVolume::FULL};
    return result;
}

void AnimatedTextureLayerBuffers::extend_aabb(ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const {
    aabb = ExtremalBoundingVolume::FULL;
}

void AnimatedTextureLayerBuffers::extend_bounding_sphere(ExtremalBoundingSphere<CompressedScenePos, 3>& bounding_sphere) const {
    bounding_sphere = ExtremalBoundingVolume::FULL;
}

void AnimatedTextureLayerBuffers::extend_aabb(
    const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
    AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const
{
    throw std::runtime_error("AnimatedTextureLayerBuffers::extend_aabb not implemented");
}

ScenePos AnimatedTextureLayerBuffers::max_center_distance2(BillboardId billboard_id) const
{
    throw std::runtime_error("AnimatedTextureLayerBuffers::max_center_distance2 not implemented");
}

std::string AnimatedTextureLayerBuffers::identifier() const {
    return mesh_meta_.name.full_name() + " (animated)";
}

void AnimatedTextureLayerBuffers::print_stats(std::ostream& ostr) const
{
    throw std::runtime_error("AnimatedTextureLayerBuffers::print_stats not implemented");
}

size_t AnimatedTextureLayerBuffers::tmp_length() const {
    return tmp_num_triangles_;
}

size_t AnimatedTextureLayerBuffers::tmp_empty() const {
    return tmp_num_triangles_ == 0;
}

size_t AnimatedTextureLayerBuffers::capacity() const {
    return max_num_triangles_;
}
