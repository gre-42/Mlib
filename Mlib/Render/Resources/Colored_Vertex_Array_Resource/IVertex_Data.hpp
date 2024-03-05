#pragma once
#include <cstddef>

namespace Mlib {

class VertexArray;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IVertexData {
public:
	virtual ~IVertexData() = default;
    virtual VertexArray& vertex_array() = 0;
    virtual const VertexArray& vertex_array() const = 0;
    virtual size_t ntriangles() const = 0;
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
