#pragma once
#include <chrono>
#include <cstddef>

namespace Mlib {

class VertexArray;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class IArrayBuffer;

class IVertexData {
public:
    virtual ~IVertexData() = default;
    virtual void update(std::chrono::steady_clock::time_point time) = 0;
    virtual void bind() const = 0;
    virtual bool copy_in_progress() const = 0;
    virtual bool initialized() const = 0;
    virtual void initialize() = 0;
    virtual void wait() const = 0;
    virtual size_t ntriangles() const = 0;
    virtual bool has_continuous_triangle_texture_layers() const = 0;
    virtual bool has_discrete_triangle_texture_layers() const = 0;
    virtual IArrayBuffer& vertex_buffer() = 0;
    virtual IArrayBuffer& bone_weight_buffer() = 0;
    virtual IArrayBuffer& texture_layer_buffer() = 0;
    virtual IArrayBuffer& interior_mapping_buffer() = 0;
    virtual IArrayBuffer& uv1_buffer(size_t i) = 0;
    virtual IArrayBuffer& cweight_buffer(size_t i) = 0;
    virtual IArrayBuffer& alpha_buffer() = 0;
    virtual void delete_triangles_far_away(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float, float, 3>& m,
        float draw_distance_add,
        float draw_distance_slop,
        size_t noperations,
        bool run_in_background,
        bool is_static) = 0;
};

}
