#pragma once
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/OpenGL/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <memory>
#include <vector>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct ColoredVertex;
class BackgroundLoop;
struct AnimatedColoredVertexArrays;

class DistantTriangleHider: public IGpuVertexData {
    DistantTriangleHider(const DistantTriangleHider&) = delete;
    DistantTriangleHider& operator = (const DistantTriangleHider&) = delete;

public:
    DistantTriangleHider(
        const std::shared_ptr<ColoredVertexArray<float>>& cva,
        const std::shared_ptr<AnimatedColoredVertexArrays>& animation,
        size_t ntriangles);
    virtual ~DistantTriangleHider() override;
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
    virtual void delete_triangles_far_away_legacy(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float, float, 3>& m,
        float draw_distance_add,
        float draw_distance_slop,
        size_t noperations,
        bool run_in_background,
        bool is_static) override;

private:
    BufferForegroundCopy vertices_;
    BufferForegroundCopy bone_weights_;
    BufferForegroundCopy texture_layers_;
    BufferForegroundCopy interior_mapping_;
    std::vector<BufferForegroundCopy> uv1_;
    std::vector<BufferForegroundCopy> cweight_;
    BufferForegroundCopy alpha_;
    std::shared_ptr<ColoredVertexArray<float>> cva_;
    std::shared_ptr<AnimatedColoredVertexArrays> animation_;
    size_t ntriangles_;

    void delete_triangle(size_t id, FixedArray<ColoredVertex<float>, 3>* ptr);
    void insert_triangle(size_t id, FixedArray<ColoredVertex<float>, 3>* ptr);

    UUVector<FixedArray<FixedArray<float, 3>, 3>> transformed_triangles_;
    std::vector<size_t> triangles_local_ids_;
    std::vector<size_t> triangles_global_ids_;
    size_t current_triangle_id_ = SIZE_MAX;
    std::unique_ptr<BackgroundLoop> background_loop_;

    std::vector<size_t> triangles_to_delete_;
    std::vector<size_t> triangles_to_insert_;

    size_t offset_;
    size_t noperations2_;
};

}
