#pragma once
#include <Mlib/Geometry/Mesh/Mesh_Meta.hpp>
#include <Mlib/OpenGL/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Dynamic_Continuous_Texture_Layer.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Dynamic_Triangle.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <chrono>
#include <cstddef>

namespace Mlib {

template <class TData, size_t tndim>
class ExtremalAxisAlignedBoundingBox;
template <class TData, size_t tndim>
class ExtremalBoundingSphere;
struct TrailSequence;
struct StaticWorld;

class AnimatedTextureLayerBuffers: public IGpuVertexData {
public:
    explicit AnimatedTextureLayerBuffers(
        size_t max_num_triangles,
        const MeshMeta& mesh_meta);

    void update(std::chrono::steady_clock::time_point time);
    virtual size_t ntriangles() const override;
    virtual size_t nuvs() const override;
    virtual size_t ncweights() const override;
    virtual bool has_alpha() const override;
    virtual bool has_continuous_triangle_texture_layers() const override;
    virtual bool has_discrete_triangle_texture_layers() const override;
    virtual bool has_interiormap() const override;
    virtual bool has_bone_indices() const override;
    virtual IArrayBuffer& vertex_buffer() override;
    virtual IArrayBuffer& bone_weight_buffer() override;
    virtual IArrayBuffer& texture_layer_buffer() override;
    virtual IArrayBuffer& interior_mapping_buffer() override;
    virtual IArrayBuffer& uv1_buffer(size_t i) override;
    virtual IArrayBuffer& cweight_buffer(size_t i) override;
    virtual IArrayBuffer& alpha_buffer() override;
    virtual void delete_triangles_far_away_legacy(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float, float, 3>& m,
        float draw_distance_add,
        float draw_distance_slop,
        size_t noperations,
        bool run_in_background,
        bool is_static) override;
    virtual const MeshMeta& mesh_meta() const override;
    virtual const ExtremalAxisAlignedBoundingBox<float, 3>& aabb() const override;
    virtual const ExtremalBoundingSphere<float, 3>& bounding_sphere() const override;
    virtual void extend_aabb(ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const override;
    virtual void extend_bounding_sphere(ExtremalBoundingSphere<CompressedScenePos, 3>& bounding_sphere) const override;
    virtual void extend_aabb(
        const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
        AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const override;
    virtual ScenePos max_center_distance2(BillboardId billboard_id) const override;
    virtual std::string identifier() const override;
    virtual void print_stats(std::ostream& ostr) const override;

    void append(
        const FixedArray<ColoredVertex<float>, 3>& triangle,
        const FixedArray<float, 3>& time,
        const TrailSequence& sequence);

    void move(float dt, const StaticWorld& world);
    std::chrono::steady_clock::time_point time() const;

    size_t tmp_length() const;
    size_t tmp_empty() const;
    size_t capacity() const;

private:
    MeshMeta mesh_meta_;
    size_t max_num_triangles_;
    size_t tmp_num_triangles_;
    GLsizei gl_num_triangles_;
    UUVector<FixedArray<float, 3>> animation_times_;
    std::vector<const TrailSequence*> animation_sequences_;
    DynamicTriangle triangle_;
    DynamicContinuousTextureLayer texture_layer_;
    std::chrono::steady_clock::time_point time_;
};

}
