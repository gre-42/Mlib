#include "Delete_Backfacing_Triangles.hpp"
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <set>

using namespace Mlib;

void Mlib::delete_backfacing_triangles(
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& lists,
    const std::string& debug_filename)
{
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> deleted_triangles;
    for (auto& l : lists) {
        l->delete_backfacing_triangles(debug_filename.empty() ? nullptr : &deleted_triangles);
    }
    if (!deleted_triangles.empty()) {
        std::set<OrderableFixedArray<CompressedScenePos, 3>> crossed_nodes;
        for (const auto& t : deleted_triangles) {
            for (const auto& v : t.flat_iterable()) {
                crossed_nodes.insert(OrderableFixedArray{v.position});
            }
        }
        std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> good_triangles;
        for (auto& l : lists) {
            for (const auto& t : l->triangles) {
                good_triangles.push_back(&t);
            }
        }
        plot_mesh(
            FixedArray<size_t, 2>{2000u, 2000u},    // image_size
            0,                                      // line_thickness
            0,                                      // point_size
            good_triangles,                         // triangles
            {},                                     // contour
            {},
            std::list<FixedArray<CompressedScenePos, 3>>(crossed_nodes.begin(), crossed_nodes.end()))
        .save_to_file(debug_filename);
    }
}
