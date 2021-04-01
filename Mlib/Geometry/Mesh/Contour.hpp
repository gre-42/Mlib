#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <map>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;

enum class ContourDetectionStrategy {
    NODE_NEIGHBOR,
    EDGE_NEIGHBOR,
    TRIANGLE
};

std::set<std::pair<OrderableFixedArray<float, 3>, OrderableFixedArray<float, 3>>>
    find_contour_edges(const std::list<const FixedArray<ColoredVertex, 3>*>& triangles);

std::set<std::pair<OrderableFixedArray<float, 3>, OrderableFixedArray<float, 3>>>
    find_contour_edges(const std::list<FixedArray<ColoredVertex, 3>*>& triangles);

std::list<std::list<FixedArray<float, 3>>> find_contours(
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    ContourDetectionStrategy strategy);

std::list<std::list<FixedArray<float, 3>>> find_contours(
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    ContourDetectionStrategy strategy);

template <class TPoint, class TTriangle>
void delete_triangles_inside_contours(
    const std::vector<std::vector<TPoint>>& contours,
    std::list<TTriangle>& triangles,
    std::vector<std::list<TTriangle>>& inner_triangles)
{
    using O = TPoint;

    inner_triangles.resize(contours.size());

    std::map<std::pair<TPoint, TPoint>, size_t> contour_edges;
    std::set<O> contour_vertices;
    std::map<O, size_t> inner_vertices;
    for (size_t contour_id = 0; contour_id < contours.size(); ++contour_id) {
        const auto& contour = contours[contour_id];
        // Convert contour path to edges, asserting that the contour is closed.
        for (auto it = contour.begin(); it != contour.end(); ++it) {
            auto s = it;
            ++s;
            if ((s == contour.end()) && (*it == contour.front())) {
                throw std::runtime_error("delete_triangles_outside_contour: Contour is closed");
            }
            if (!contour_edges.insert(
                std::make_pair(
                    std::make_pair(O{*it}, O{s == contour.end() ? contour.front() : *s}),
                    contour_id)).second)
            {
                throw std::runtime_error("Could not insert contour edge");
            }
        }
        // Delete inside triangles touching a contour,
        // and add their inner vertices to the "inner_vertices" set.
        triangles.remove_if([contour_id, &contour_edges, &inner_vertices, &inner_triangles](const TTriangle& t){
            for (size_t i = 0; i < 3; ++i) {
                const auto& a = t(i);
                const auto& b = t((i + 1) % 3);
                const auto& c = t((i + 2) % 3);
                auto it = contour_edges.find(std::make_pair(O{b}, O{a}));
                if (it != contour_edges.end()) {
                    // ignore result
                    inner_vertices.insert(std::make_pair(O{c}, it->second));
                    inner_triangles[contour_id].push_back(t);
                    return true;
                }
            }
            return false;
        });
        contour_vertices.insert(contour.begin(), contour.end());
    }
    // Add all vertices touching an inside vertex
    // to the "inner_vertices" set,
    // and loop until nothing changes.
    while(true) {
        size_t old_size = inner_vertices.size();
        for (const auto& t : triangles) {
            for (size_t iv = 0; iv < 3; ++iv) {
                auto it = inner_vertices.find(O{t(iv)});
                if (it != inner_vertices.end()) {
                    for (size_t iv1 = 0; iv1 < 3; ++iv1) {
                        if (!contour_vertices.contains(O{t(iv1)})) {
                            inner_vertices.insert(std::make_pair(O{t(iv1)}, it->second));
                        }
                    }
                }
            }
        }
        if (inner_vertices.size() == old_size) {
            break;
        }
    }
    // Delete all triangles containing an inner vertex.
    triangles.remove_if([&inner_vertices, &inner_triangles](const TTriangle& t){
        for (size_t i = 0; i < 3; ++i) {
            const auto a = inner_vertices.find(O{t(i)});
            if (a != inner_vertices.end()) {
                inner_triangles[a->second].push_back(t);
                return true;
            }
        }
        return false;
    });
}

}
