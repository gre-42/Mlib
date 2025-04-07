#include "Distant_Triangle_Hider.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

using namespace Mlib;

DistantTriangleHider::DistantTriangleHider(
    std::shared_ptr<ColoredVertexArray<float>> cva,
    size_t ntriangles,
    std::shared_ptr<IArrayBuffer> inherited_vertices)
    : inherited_vertices_{ std::move(inherited_vertices) }
    , vertices_{ inherited_vertices_ == nullptr ? std::make_shared<BufferBackgroundCopy>() : nullptr }
    , vertex_buffer_{ inherited_vertices_ == nullptr ? *vertices_ : *inherited_vertices_ }
    , uv1_( cva->uv1.size() )
    , cweight_{ cva->cweight.size() }
    , cva_{ std::move(cva) }
    , ntriangles_{ ntriangles }
{
    va_.add_array_buffer(vertex_buffer_);
    va_.add_array_buffer(bone_weights_);
    va_.add_array_buffer(texture_layers_);
    va_.add_array_buffer(interior_mapping_);
    for (auto& u : uv1_) {
        va_.add_array_buffer(u);
    }
    for (auto& c : cweight_) {
        va_.add_array_buffer(c);
    }
    va_.add_array_buffer(alpha_);
}

void DistantTriangleHider::update_legacy() {
    va_.update();
}

void DistantTriangleHider::bind() const {
    va_.bind();
}

bool DistantTriangleHider::copy_in_progress() const {
    return va_.copy_in_progress();
}

bool DistantTriangleHider::initialized() const {
    return va_.initialized();
}

void DistantTriangleHider::initialize() {
    va_.initialize();
}

void DistantTriangleHider::wait() const {
    va_.wait();
}

size_t DistantTriangleHider::ntriangles() const {
    return ntriangles_;
}

bool DistantTriangleHider::has_continuous_triangle_texture_layers() const {
    return !cva_->continuous_triangle_texture_layers.empty();
}

bool DistantTriangleHider::has_discrete_triangle_texture_layers() const {
    return !cva_->discrete_triangle_texture_layers.empty();
}

IArrayBuffer& DistantTriangleHider::vertex_buffer() {
    return vertex_buffer_;
}

IArrayBuffer& DistantTriangleHider::bone_weight_buffer() {
    return bone_weights_;
}

IArrayBuffer& DistantTriangleHider::texture_layer_buffer() {
    return texture_layers_;
}

IArrayBuffer& DistantTriangleHider::interior_mapping_buffer() {
    return interior_mapping_;
}

IArrayBuffer& DistantTriangleHider::uv1_buffer(size_t i) {
    if (i >= uv1_.size()) {
        THROW_OR_ABORT("UV1 index too large");
    }
    return uv1_[i];
}

IArrayBuffer& DistantTriangleHider::cweight_buffer(size_t i) {
    if (i >= cweight_.size()) {
        THROW_OR_ABORT("cweight index too large");
    }
    return cweight_[i];
}

IArrayBuffer& DistantTriangleHider::alpha_buffer() {
    return alpha_;
}

void DistantTriangleHider::delete_triangle(size_t id, FixedArray<ColoredVertex<float>, 3>* ptr) {
    // assert(ntriangles > 0);
    if (triangles_local_ids_[id] == ntriangles_ - 1) {
        triangles_local_ids_[id] = SIZE_MAX;
        triangles_global_ids_[ntriangles_ - 1] = SIZE_MAX;
    } else if (ntriangles_ > 1) {
        size_t from = ntriangles_ - 1;
        size_t to = triangles_local_ids_[id];
        assert(to != SIZE_MAX);
        ptr[to] = cva_->triangles[triangles_global_ids_[from]];
        triangles_local_ids_[id] = SIZE_MAX;
        triangles_local_ids_[triangles_global_ids_[from]] = to;
        triangles_global_ids_[to] = triangles_global_ids_[from];
        triangles_global_ids_[from] = SIZE_MAX;
    }
    --ntriangles_;
}

void DistantTriangleHider::insert_triangle(size_t id, FixedArray<ColoredVertex<float>, 3>* ptr) {
    // assert(triangles_global_ids_[ntriangles] == SIZE_MAX);
    assert(triangles_local_ids_[id] == SIZE_MAX);
    triangles_local_ids_[id] = ntriangles_;
    triangles_global_ids_[ntriangles_] = id;
    ptr[ntriangles_] = cva_->triangles[id];
    ++ntriangles_;
}

/**
 * From: https://learnopengl.com/Advanced-OpenGL/Advanced-Data
 */
void DistantTriangleHider::delete_triangles_far_away_legacy(
    const FixedArray<float, 3>& position,
    const TransformationMatrix<float, float, 3>& m,
    float draw_distance_add,
    float draw_distance_slop,
    size_t noperations,
    bool run_in_background,
    bool is_static)
{
    assert_true(!std::isnan(draw_distance_add));
    assert_true(!std::isnan(draw_distance_slop));
    if (draw_distance_add == INFINITY) {
        return;
    }
    // TimeGuard tg{ "delete_triangles_far_away", "delete_triangles_far_away" };
    if (cva_->triangles.empty()) {
        return;
    }
    auto update_counters = [this, noperations](){
        offset_ = current_triangle_id_;
        noperations2_ = std::min(current_triangle_id_ + noperations, cva_->triangles.size()) - current_triangle_id_;
    };
    if (triangles_local_ids_.empty()) {
        triangles_local_ids_.resize(cva_->triangles.size());
        triangles_global_ids_.resize(cva_->triangles.size());
        for (size_t i = 0; i < triangles_local_ids_.size(); ++i) {
            triangles_local_ids_[i] = i;
            triangles_global_ids_[i] = i;
        }
        current_triangle_id_ = 0;
        update_counters();
        if (is_static) {
            transformed_triangles_.resize(cva_->triangles.size());
            for (size_t i = 0; i < cva_->triangles.size(); ++i) {
                transformed_triangles_[i] = cva_->triangles[i].applied<FixedArray<float, 3>>([&m](const ColoredVertex<float>& v){return m.transform(v.position);});
            }
        }
    }
    if (triangles_local_ids_.size() != cva_->triangles.size()) {
        THROW_OR_ABORT("Array length has changed");
    }
    using Triangle = FixedArray<ColoredVertex<float>, 3>;
    auto func = [this, draw_distance_add, draw_distance_slop, m, position, run_in_background, is_static](){
        Triangle* ptr = nullptr;
        if (!run_in_background) {
            // Must be inside here because CHK requires an OpenGL context
#ifdef __ANDROID__
            THROW_OR_ABORT("Triangle replacement not supported on Android");
#else
            CHK(ptr = (Triangle*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
#endif
        }
        float add2 = squared(draw_distance_add);
        float remove2 = squared(draw_distance_add + draw_distance_slop);
        for (size_t i = 0; i < noperations2_; ++i) {
            FixedArray<float, 3> center = uninitialized;
            if (is_static) {
                center = mean(transformed_triangles_[current_triangle_id_]);
            } else {
                auto center_local = mean(cva_->triangles[current_triangle_id_].applied<FixedArray<float, 3>>([](const ColoredVertex<float>& c){return c.position;}));
                center = m.transform(center_local);
            }
            float dist2 = sum(squared(center - position));
            if (triangles_local_ids_[current_triangle_id_] != SIZE_MAX) {
                if (dist2 > remove2) {
                    if (run_in_background) {
                        if (triangles_to_delete_.size() == triangles_to_delete_.capacity()) {
                            return;
                        }
                        triangles_to_delete_.push_back(current_triangle_id_);
                    } else {
                        delete_triangle(current_triangle_id_, ptr);
                    }
                }
            } else {
                if (dist2 < add2) {
                    if (run_in_background) {
                        if (triangles_to_insert_.size() == triangles_to_insert_.capacity()) {
                            return;
                        }
                        triangles_to_insert_.push_back(current_triangle_id_);
                    } else {
                        insert_triangle(current_triangle_id_, ptr);
                    }
                }
            }
            current_triangle_id_ = (current_triangle_id_ + 1) % triangles_local_ids_.size();
        }
    };
    if (run_in_background) {
        if (background_loop_ == nullptr) {
            background_loop_ = std::make_unique<BackgroundLoop>("Subst_BG");
            triangles_to_delete_.reserve(noperations2_);
            triangles_to_insert_.reserve(noperations2_);
        }
        if (triangles_to_delete_.capacity() < noperations2_) {
            THROW_OR_ABORT("noperations or triangle list changed");
        }
        if (background_loop_->done()) {
            if (!triangles_to_delete_.empty() || !triangles_to_insert_.empty()) {
                // TimeGuard tg{ "deleting triangles", "deleting triangles" };
                vertex_buffer_.bind();
                // CHK(auto* ptr = (Triangle*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                CHK(auto* ptr = (Triangle*)glMapBufferRange(GL_ARRAY_BUFFER, integral_cast<GLintptr>(offset_ * sizeof(Triangle)), integral_cast<GLsizeiptr>(noperations2_ * sizeof(Triangle)), GL_MAP_WRITE_BIT));
                ptr -= offset_;
                for (size_t i : triangles_to_delete_) {
                    delete_triangle(i, ptr);
                }
                triangles_to_delete_.clear();
                for (size_t i : triangles_to_insert_) {
                    insert_triangle(i, ptr);
                }
                triangles_to_insert_.clear();
                CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
            }
            update_counters();
            background_loop_->run(func);
        }
    } else {
        if (background_loop_ != nullptr) {
            THROW_OR_ABORT("Substitution both in fg and bg");
        }
        update_counters();
        vertex_buffer_.bind();
        func();
        CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
    }
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
