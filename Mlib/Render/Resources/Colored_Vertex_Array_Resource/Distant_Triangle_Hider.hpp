#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IVertex_Data.hpp>
#include <memory>
#include <vector>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct ColoredVertex;
class BackgroundLoop;

class DistantTriangleHider: public IVertexData {
    DistantTriangleHider(const DistantTriangleHider&) = delete;
    DistantTriangleHider& operator = (const DistantTriangleHider&) = delete;

public:
    DistantTriangleHider(
        std::shared_ptr<ColoredVertexArray<float>> cva,
        size_t ntriangles,
        std::shared_ptr<IArrayBuffer> inherited_vertices);
    virtual void update_legacy() override;
    virtual void bind() const override;
    virtual bool copy_in_progress() const override;
    virtual bool initialized() const override;
    virtual void initialize() override;
    virtual void wait() const override;
    virtual size_t ntriangles() const override;
    virtual bool has_continuous_triangle_texture_layers() const override;
    virtual bool has_discrete_triangle_texture_layers() const override;
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

private:
    std::shared_ptr<IArrayBuffer> inherited_vertices_;
    std::shared_ptr<BufferBackgroundCopy> vertices_;
    IArrayBuffer& vertex_buffer_;
    BufferBackgroundCopy bone_weights_;
    BufferBackgroundCopy texture_layers_;
    BufferBackgroundCopy interior_mapping_;
    std::vector<BufferBackgroundCopy> uv1_;
    std::vector<BufferBackgroundCopy> cweight_;
    BufferBackgroundCopy alpha_;
    VertexArray va_;
    std::shared_ptr<ColoredVertexArray<float>> cva_;
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
