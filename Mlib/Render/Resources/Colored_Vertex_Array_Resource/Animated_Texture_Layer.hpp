#pragma once
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Continuous_Texture_Layer.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Triangle.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Empty_Array_Buffer.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IVertex_Data.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>

namespace Mlib {

struct TrailSequence;

class AnimatedTextureLayer: public IVertexData {
public:
    explicit AnimatedTextureLayer(size_t max_num_triangles);

    virtual void update() override;
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
    virtual void delete_triangles_far_away(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float, float, 3>& m,
        float draw_distance_add,
        float draw_distance_slop,
        size_t noperations,
        bool run_in_background,
        bool is_static) override;

    void append(
        const FixedArray<ColoredVertex<float>, 3>& triangle,
        const FixedArray<float, 3>& time,
        const TrailSequence& sequence);

    void move(float dt);

    size_t tmp_length() const;
    size_t tmp_empty();
    size_t capacity() const;

private:
    size_t max_num_triangles_;
    size_t tmp_num_triangles_;
    GLsizei gl_num_triangles_;
    std::vector<FixedArray<float, 3>> animation_times_;
    std::vector<const TrailSequence*> animation_sequences_;
    DynamicTriangle triangle_;
    DynamicContinuousTextureLayer texture_layer_;
    EmptyArrayBuffer empty_;
    VertexArray va_;
    mutable SafeSharedMutex mutex_;
};

}