#pragma once
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <memory>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct ColoredVertex;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class BackgroundLoop;

class SubstitutionInfo {
public:
    VertexArray va_;
    std::shared_ptr<ColoredVertexArray<float>> cva_;
    size_t ntriangles_;
    size_t nlines_;
    void delete_triangles_far_away(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float, float, 3>& m,
        float draw_distance_add,
        float draw_distance_slop,
        size_t noperations,
        bool run_in_background,
        bool is_static);
private:
    void delete_triangle(size_t id, FixedArray<ColoredVertex<float>, 3>* ptr);
    void insert_triangle(size_t id, FixedArray<ColoredVertex<float>, 3>* ptr);

    std::vector<FixedArray<FixedArray<float, 3>, 3>> transformed_triangles_;
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
