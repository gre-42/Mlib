#pragma once
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <memory>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class ColoredVertexArray;
class ColoredVertex;
template <class TData>
class TransformationMatrix;

struct SubstitutionInfo {
    VertexArray va;
    std::shared_ptr<ColoredVertexArray> cva;
    size_t ntriangles;
    size_t nlines;
    std::vector<size_t> triangles_local_ids;
    std::vector<size_t> triangles_global_ids;
    size_t current_triangle_id = SIZE_MAX;
    void delete_triangle(size_t id, FixedArray<ColoredVertex, 3>* ptr);
    void insert_triangle(size_t id, FixedArray<ColoredVertex, 3>* ptr);
    void delete_triangles_far_away(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float>& m,
        float draw_distance_add,
        float draw_distance_slop,
        size_t noperations);
};

}
