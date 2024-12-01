#include "Animated_Texture_Layer.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Sequence.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <mutex>

using namespace Mlib;

AnimatedTextureLayer::AnimatedTextureLayer(size_t max_num_triangles)
    : max_num_triangles_{ max_num_triangles }
    , tmp_num_triangles_{ 0 }
    , gl_num_triangles_{ 0 }
    , animation_times_(max_num_triangles)
    , animation_sequences_(max_num_triangles)
    , triangle_{ max_num_triangles }
    , texture_layer_{ max_num_triangles }
{
    va_.add_array_buffer(triangle_);
    va_.add_array_buffer(texture_layer_);
}

void AnimatedTextureLayer::update(std::chrono::steady_clock::time_point time) {
    if (time == std::chrono::steady_clock::time_point()) {
        THROW_OR_ABORT("AnimatedTextureLayer::update received uninitialized time");
    }
    std::scoped_lock lock{ mutex_ };
    if (time_ == std::chrono::steady_clock::time_point()) {
        if (tmp_num_triangles_ != 0) {
            THROW_OR_ABORT("AnimatedTextureLayer::update without previous move");
        }
    } else {
        auto time_offset = std::chrono::duration<float>(time - time_).count() * seconds;
        for (size_t i = 0; i < tmp_length(); ++i) {
            const auto& ai = animation_times_[i];
            const auto& bi = *animation_sequences_[i];
            texture_layer_[i] = ai.applied([&](const auto& v) { return bi.times_to_w(v + time_offset); });
        }
    }
    va_.update();
    gl_num_triangles_ = integral_cast<GLsizei>(tmp_num_triangles_);
}

void AnimatedTextureLayer::bind() const {
    va_.bind();
}

bool AnimatedTextureLayer::copy_in_progress() const {
    if (va_.copy_in_progress()) {
        verbose_abort("AnimatedTextureLayer::copy_in_progress internal error");
    }
    return false;
}

bool AnimatedTextureLayer::initialized() const {
    return va_.initialized();
}

void AnimatedTextureLayer::initialize() {
    va_.initialize();
}

void AnimatedTextureLayer::wait() const {
    // Do nothing
}

size_t AnimatedTextureLayer::ntriangles() const {
    return integral_cast<size_t>(gl_num_triangles_);
}

bool AnimatedTextureLayer::has_continuous_triangle_texture_layers() const {
    return true;
}

bool AnimatedTextureLayer::has_discrete_triangle_texture_layers() const {
    return false;
}

IArrayBuffer& AnimatedTextureLayer::vertex_buffer() {
    return triangle_;
}

IArrayBuffer& AnimatedTextureLayer::bone_weight_buffer() {
    THROW_OR_ABORT("AnimatedTextureLayer has no bone_weight_buffer");
}

IArrayBuffer& AnimatedTextureLayer::texture_layer_buffer() {
    return texture_layer_;
}

IArrayBuffer& AnimatedTextureLayer::interior_mapping_buffer() {
    THROW_OR_ABORT("AnimatedTextureLayer has no interior_mapping_buffer");
}

IArrayBuffer& AnimatedTextureLayer::uv1_buffer(size_t i) {
    THROW_OR_ABORT("AnimatedTextureLayer has no uv1_buffer");
}

IArrayBuffer& AnimatedTextureLayer::cweight_buffer(size_t i) {
    THROW_OR_ABORT("AnimatedTextureLayer has no cweight_buffer");
}

IArrayBuffer& AnimatedTextureLayer::alpha_buffer() {
    THROW_OR_ABORT("AnimatedTextureLayer has no alpha_buffer");
}

void AnimatedTextureLayer::append(
    const FixedArray<ColoredVertex<float>, 3>& triangle,
    const FixedArray<float, 3>& time,
    const TrailSequence& sequence)
{
    std::scoped_lock lock{ mutex_ };
    if (tmp_num_triangles_ >= max_num_triangles_) {
        THROW_OR_ABORT("Maximum number of triangles exceeded");
    }
    triangle_.append(triangle);
    texture_layer_.append(fixed_nans<float, 3>());
    animation_times_[tmp_num_triangles_] = time;
    animation_sequences_[tmp_num_triangles_] = &sequence;
    ++tmp_num_triangles_;
}

void AnimatedTextureLayer::move(float dt, const StaticWorld& world) {
    std::scoped_lock lock{ mutex_ };
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

std::chrono::steady_clock::time_point AnimatedTextureLayer::time() const {
    return time_;
}

void AnimatedTextureLayer::delete_triangles_far_away(
    const FixedArray<float, 3>& position,
    const TransformationMatrix<float, float, 3>& m,
    float draw_distance_add,
    float draw_distance_slop,
    size_t noperations,
    bool run_in_background,
    bool is_static)
{
    THROW_OR_ABORT("AnimatedTextureLayer::delete_triangles_far_away not implemented");
}

size_t AnimatedTextureLayer::tmp_length() const {
    return tmp_num_triangles_;
}

size_t AnimatedTextureLayer::tmp_empty() const {
    return tmp_num_triangles_ == 0;
}

size_t AnimatedTextureLayer::capacity() const {
    return max_num_triangles_;
}
