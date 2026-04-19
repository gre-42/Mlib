#include "Distant_Triangle_Hider.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Misc/Pragma_Gcc.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/OpenGL/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Shader_Structs.hpp>
#include <Mlib/Scene_Graph/Render/Attribute_Index_Calculator.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <stdexcept>

PRAGMA_GCC(push_options)
PRAGMA_GCC(optimize ("O3"))

using namespace Mlib;

DistantTriangleHider::DistantTriangleHider(
    const std::shared_ptr<ColoredVertexArray<float>>& cva,
    const std::shared_ptr<AnimatedColoredVertexArrays>& animation,
    size_t ntriangles)
    : uv1_( cva->uv1.size() )
    , cweight_{ cva->cweight.size() }
    , cva_{ cva }
    , animation_{ animation }
    , ntriangles_{ ntriangles }
{}

DistantTriangleHider::~DistantTriangleHider() = default;

size_t DistantTriangleHider::ntriangles() const {
    return ntriangles_;
}

size_t DistantTriangleHider::nuvs() const {
    return cva_->uv1.size() + 1;
}

size_t DistantTriangleHider::ncweights() const {
    return cva_->cweight.size();
}

bool DistantTriangleHider::has_alpha() const {
    return !cva_->alpha.empty();
}

bool DistantTriangleHider::has_continuous_triangle_texture_layers() const {
    return !cva_->continuous_triangle_texture_layers.empty();
}

bool DistantTriangleHider::has_discrete_triangle_texture_layers() const {
    return !cva_->discrete_triangle_texture_layers.empty();
}

bool DistantTriangleHider::has_interiormap() const {
    return !cva_->meta.material.interior_textures.empty();
}

bool DistantTriangleHider::has_bone_indices() const {
    assert_true(cva_->triangle_bone_weights.empty() == !(animation_ && animation_->skeleton));
    return !cva_->triangle_bone_weights.empty();
}

IArrayBuffer& DistantTriangleHider::vertex_buffer() {
    if (!vertices_.is_initialized()) {
        vertices_.init(cva_->triangles);
    }
    return vertices_;
}

IArrayBuffer& DistantTriangleHider::bone_weight_buffer() {
    assert_true(cva_->triangle_bone_weights.empty() == !(animation_ && animation_->skeleton));
    if (!cva_->triangle_bone_weights.empty() && !bone_weights_.is_initialized()) {
        UUVector<FixedArray<ShaderBoneWeight, 3>> triangle_bone_weights(cva_->triangle_bone_weights.size());
        for (size_t tid = 0; tid < triangle_bone_weights.size(); ++tid) {
            const auto& td = cva_->triangle_bone_weights[tid];  // std::vector of bone weights.
            auto& ts = triangle_bone_weights[tid];             // FixedArray of sorted bone weights.
            for (size_t vid = 0; vid < CW::length(td); ++vid) {
                auto vd = td(vid);   // Copy of std::vector of bone weights, to be sorted.
                auto& vs = ts(vid);  // Reference to FixedArray of sorted bone weights.
                // Sort in descending order
                std::sort(vd.begin(), vd.end(), [](const BoneWeight& w0, const BoneWeight& w1){return w0.weight > w1.weight;});
                float sum_weights = 0;
                for (size_t i = 0; i < ANIMATION_NINTERPOLATED; ++i) {
                    if (i < vd.size()) {
                        if (vd[i].bone_index >= animation_->bone_indices.size()) {
                            throw std::runtime_error(
                                "Bone index too large in get_vertex_array: " +
                                std::to_string(vd[i].bone_index) + " >= " +
                                std::to_string(animation_->bone_indices.size()));
                        }
                        if (vd[i].bone_index > 255) {
                            throw std::runtime_error("Bone index too large for unsigned char");
                        }
                        vs.indices[i] = (unsigned char)vd[i].bone_index;
                        vs.weights[i] = vd[i].weight;
                        sum_weights += vs.weights[i];
                    } else {
                        vs.indices[i] = 0;
                        vs.weights[i] = 0;
                    }
                }
                if (sum_weights < 1e-3) {
                    throw std::runtime_error("Sum of weights too small");
                }
                if (sum_weights > 1.1) {
                    throw std::runtime_error("Sum of weights too large");
                }
                for (float& weight : vs.weights) {
                    weight /= sum_weights;
                }
            }
        }
        // The "triangle_bone_weights" array is temporary, so wait until it is transferred.
        bone_weights_.init(triangle_bone_weights);
        bone_weights_.wait();
    }
    return bone_weights_;
}

IArrayBuffer& DistantTriangleHider::texture_layer_buffer() {
    if (!texture_layers_.is_initialized())
    {
        if (has_continuous_triangle_texture_layers()) {
            if (cva_->continuous_triangle_texture_layers.size() != cva_->triangles.size()) {
                throw std::runtime_error("#continuous_triangle_texture_layers != #triangles");
            }
            texture_layers_.init(cva_->continuous_triangle_texture_layers);
        }
        if (has_discrete_triangle_texture_layers()) {
            if (cva_->discrete_triangle_texture_layers.size() != cva_->triangles.size()) {
                throw std::runtime_error("#discrete_triangle_texture_layers != #triangles");
            }
            texture_layers_.init(cva_->discrete_triangle_texture_layers);
        }
    }
    return texture_layers_;
}

IArrayBuffer& DistantTriangleHider::interior_mapping_buffer() {
    if (!interior_mapping_.is_initialized()) {
        if (cva_->triangles.size() != cva_->interiormap_uvmaps.size()) {
            throw std::runtime_error(
                (std::stringstream() << "#triangles: " << cva_->triangles.size() <<
                ", #interiormap_uvmap: " << cva_->interiormap_uvmaps.size()).str());
        }
        std::vector<ShaderInteriorMappedFacade> shader_interior_mapped_facade;
        shader_interior_mapped_facade.reserve(3 * cva_->triangles.size());
        for (const auto& [i, t] : enumerate(cva_->triangles)) {
            for (size_t j = 0; j < 3; ++j) {
                shader_interior_mapped_facade.push_back(ShaderInteriorMappedFacade{
                    .bottom_left = t(0).position,
                    .uvmap = cva_->interiormap_uvmaps[i]
                    });
            }
        }
        // The "shader_interior_mapped_facade" array is temporary, so wait until it is transferred.
        interior_mapping_.init(shader_interior_mapped_facade);
        interior_mapping_.wait();
    }
    return interior_mapping_;
}

IArrayBuffer& DistantTriangleHider::uv1_buffer(size_t i) {
    if (i >= uv1_.size()) {
        throw std::runtime_error("UV1 index too large");
    }
    if (!uv1_[i].is_initialized()) {
        if (cva_->uv1.size() != uv1_.size()) {
            throw std::runtime_error("#uv1 != #buffers");
        }
        if (cva_->uv1[i].size() != cva_->triangles.size()) {
            throw std::runtime_error("#uv1 != #triangles");
        }
        uv1_[i].init(cva_->uv1[i]);
    }
    return uv1_[i];
}

IArrayBuffer& DistantTriangleHider::cweight_buffer(size_t i) {
    if (i >= cweight_.size()) {
        throw std::runtime_error("cweight index too large");
    }
    if (!cweight_[i].is_initialized()) {
        if (cva_->cweight.size() != cweight_.size()) {
            throw std::runtime_error("Unexpected number of weight coefficients");
        }
        if (cva_->cweight[i].size() != cva_->triangles.size()) {
            throw std::runtime_error("#cweights != #triangles");
        }
        cweight_[i].init(cva_->cweight[i]);
    }
    return cweight_[i];
}

IArrayBuffer& DistantTriangleHider::alpha_buffer() {
    if (!alpha_.is_initialized()) {
        if (cva_->alpha.size() != cva_->triangles.size()) {
            throw std::runtime_error("#alpha != #triangles");
        }
        alpha_.init(cva_->alpha);
    }
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
        throw std::runtime_error("Array length has changed");
    }
    using Triangle = FixedArray<ColoredVertex<float>, 3>;
    auto func = [this, draw_distance_add, draw_distance_slop, m, position, run_in_background, is_static](){
        Triangle* ptr = nullptr;
        if (!run_in_background) {
            // Must be inside here because CHK requires an OpenGL context
#ifdef __ANDROID__
            throw std::runtime_error("Triangle replacement not supported on Android");
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
            throw std::runtime_error("noperations or triangle list changed");
        }
        if (background_loop_->done()) {
            if (!triangles_to_delete_.empty() || !triangles_to_insert_.empty()) {
                // TimeGuard tg{ "deleting triangles", "deleting triangles" };
                vertices_.bind();
                // CHK(auto* ptr = (Triangle*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                auto* ptr = (Triangle*)CHK_X(glMapBufferRange(GL_ARRAY_BUFFER, integral_cast<GLintptr>(offset_ * sizeof(Triangle)), integral_cast<GLsizeiptr>(noperations2_ * sizeof(Triangle)), GL_MAP_WRITE_BIT));
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
            throw std::runtime_error("Substitution both in fg and bg");
        }
        update_counters();
        vertices_.bind();
        func();
        CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
    }
}

const MeshMeta& DistantTriangleHider::mesh_meta() const {
    return cva_->meta;
}

const ExtremalAxisAlignedBoundingBox<float, 3>& DistantTriangleHider::aabb() const {
    return cva_->aabb();
}

const ExtremalBoundingSphere<float, 3>& DistantTriangleHider::bounding_sphere() const {
    return cva_->bounding_sphere();
}

void DistantTriangleHider::extend_aabb(ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const {
    cva_->extend_aabb(aabb);
}

void DistantTriangleHider::extend_bounding_sphere(ExtremalBoundingSphere<CompressedScenePos, 3>& bounding_sphere) const {
    cva_->extend_bounding_sphere(bounding_sphere);
}

void DistantTriangleHider::extend_aabb(
    const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
    AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const
{
    cva_->extend_aabb(mv, aabb);
}

ScenePos DistantTriangleHider::max_center_distance2(BillboardId billboard_id) const {
    return cva_->max_center_distance2(billboard_id);
}

std::string DistantTriangleHider::identifier() const {
    return cva_->identifier();
}

void DistantTriangleHider::print_stats(std::ostream& ostr) const {
    cva_->print_stats(ostr);
}

PRAGMA_GCC(pop_options)
