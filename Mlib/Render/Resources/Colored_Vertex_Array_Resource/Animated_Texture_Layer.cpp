#include "Animated_Texture_Layer.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Sequence.hpp>
#include <mutex>

using namespace Mlib;

AnimatedTextureLayer::AnimatedTextureLayer(size_t max_num_triangles)
    : max_num_triangles_{ max_num_triangles }
    , tmp_num_triangles_{ 0 }
    , gl_num_triangles_{ 0 }
    , triangle_{ max_num_triangles }
    , texture_layer_{ max_num_triangles }
    , va_{
        triangle_,
        empty_,
        texture_layer_,
        empty_}
{}

VertexArray& AnimatedTextureLayer::vertex_array() {
    return va_;
}

const VertexArray& AnimatedTextureLayer::vertex_array() const {
    return va_;
}

size_t AnimatedTextureLayer::ntriangles() const {
    return gl_num_triangles_;
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
    texture_layer_.append(time.applied([&sequence](const auto& v) { return sequence.times_to_frames(v); }));
    ++tmp_num_triangles_;
}

void AnimatedTextureLayer::move(float dt) {
    std::scoped_lock lock{ mutex_ };
    for (size_t i = 0; i < tmp_length();) {
        auto& ai = animation_times_[i];
        auto& bi = animation_sequences_[i];
        ai += fixed_full<float, 3>(dt);
        if (any(ai.applied<bool>([&bi](const auto& t){ return bi->times_to_frames.is_within_range(t); }))) {
            auto& tl = texture_layer_[i];
            for (size_t j = 0; j < 3; ++j) {
                tl(j) = bi->times_to_frames(ai(j));
            }
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

size_t AnimatedTextureLayer::tmp_empty() {
    return tmp_num_triangles_ == 0;
}

size_t AnimatedTextureLayer::capacity() const {
    return max_num_triangles_;
}
