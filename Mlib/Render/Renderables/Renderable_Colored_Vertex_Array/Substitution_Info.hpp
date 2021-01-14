#pragma once
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <memory>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class ColoredVertexArray;
class ColoredVertex;
template <class TData, size_t tsize>
class TransformationMatrix;
class BackgroundLoop;

class SubstitutionInfo {
public:
    VertexArray va;
    std::shared_ptr<ColoredVertexArray> cva;
    size_t ntriangles;
    size_t nlines;
    void delete_triangles_far_away(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float, 3>& m,
        float draw_distance_add,
        float draw_distance_slop,
        size_t noperations,
        bool run_in_background,
        bool is_static);
private:
    void delete_triangle(size_t id, FixedArray<ColoredVertex, 3>* ptr);
    void insert_triangle(size_t id, FixedArray<ColoredVertex, 3>* ptr);

    std::vector<FixedArray<FixedArray<float, 3>, 3>> transformed_triangles;
    std::vector<size_t> triangles_local_ids;
    std::vector<size_t> triangles_global_ids;
    size_t current_triangle_id = SIZE_MAX;
    std::unique_ptr<BackgroundLoop> background_loop_;

    std::vector<size_t> triangles_to_delete_;
    std::vector<size_t> triangles_to_insert_;
};

}
