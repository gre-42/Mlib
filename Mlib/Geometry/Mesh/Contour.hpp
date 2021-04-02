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
    std::map<std::pair<TPoint, TPoint>, size_t> inner_edges;
    // Convert contour pathes to edges, asserting that the contours are closed.
    for (size_t contour_id = 0; contour_id < contours.size(); ++contour_id) {
        const auto& contour = contours[contour_id];
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
    }
    // Delete inside triangles having a contour edge,
    // and add their inner edges to the "inner_edges" set.
    triangles.remove_if([&contour_edges, &inner_edges, &inner_triangles](const TTriangle& t){
        for (size_t i = 0; i < 3; ++i) {
            const auto& a = t(i);
            const auto& b = t((i + 1) % 3);
            const auto& c = t((i + 2) % 3);
            auto it = contour_edges.find(std::make_pair(O{a}, O{b}));
            if (it != contour_edges.end()) {
                if (auto bc = std::make_pair(O{b}, O{c}); contour_edges.find(bc) == contour_edges.end()) {
                    inner_edges.insert(std::make_pair(bc, it->second));
                }
                if (auto ca = std::make_pair(O{c}, O{a}); contour_edges.find(ca) == contour_edges.end()) {
                    inner_edges.insert(std::make_pair(ca, it->second));
                }
                inner_triangles[it->second].push_back(t);
                return true;
            }
        }
        return false;
    });
    // Add all vertices touching an inside vertex
    // to the "inner_vertices" set,
    // and loop until nothing changes.
    while(true) {
        size_t old_size = inner_edges.size();
        triangles.remove_if([&inner_edges, &inner_triangles, &contour_edges](const TTriangle& t){
            size_t contour_id = SIZE_MAX;
            for (size_t i = 0; i < 3; ++i) {
                const auto& a = t(i);
                const auto& b = t((i + 1) % 3);
                auto ba = std::make_pair(O{b}, O{a});
                auto it = inner_edges.find(ba);
                if (it != inner_edges.end()) {
                    if (contour_id == SIZE_MAX) {
                        contour_id = it->second;
                    } else {
                        if (contour_id != it->second) {
                            throw std::runtime_error("Could not determine contour ID");
                        }
                    }
                }
            }
            if (contour_id != SIZE_MAX) {
                for (size_t i = 0; i < 3; ++i) {
                    const auto& a = t(i);
                    const auto& b = t((i + 1) % 3);
                    auto ab = std::make_pair(O{a}, O{b});
                    if (contour_edges.find(ab) == contour_edges.end()) {
                        inner_edges.insert(std::make_pair(ab, contour_id));
                    }
                }
                inner_triangles[contour_id].push_back(t);
                return true;
            }
            return false;
        });
        if (inner_edges.size() == old_size) {
            break;
        }
    }
}

}
