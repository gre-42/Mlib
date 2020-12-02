#include "Contour.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

std::list<std::list<FixedArray<float, 3>>> Mlib::find_contours(
    const std::list<FixedArray<ColoredVertex, 3>>& triangles)
{
    using O = OrderableFixedArray<float, 3>;

    std::set<std::pair<O, O>> edges;
    std::map<O, O> neighbors;
    for(const auto& t : triangles) {
        auto safe_insert_edge = [&edges, &t](size_t a, size_t b){
            auto v = std::make_pair(O(t(a).position), O(t(b).position));
            if (!edges.insert(v).second) {
                throw std::runtime_error("Detected duplicate edge");
            }
        };
        safe_insert_edge(0, 1);
        safe_insert_edge(1, 2);
        safe_insert_edge(2, 0);
    }
    for(const auto& t : triangles) {
        edges.erase(std::make_pair(O(t(1).position), O(t(0).position)));
        edges.erase(std::make_pair(O(t(2).position), O(t(1).position)));
        edges.erase(std::make_pair(O(t(0).position), O(t(2).position)));
    }
    for(const auto& t : triangles) {
        auto safe_insert_neighbor = [&edges, &neighbors, &t](size_t a, size_t b) {
            auto v = std::make_pair(O(t(a).position), O(t(b).position));
            if (edges.find(v) != edges.end()) {
                if (!neighbors.insert(v).second) {
                    throw std::runtime_error("Contour neighbor already set");
                }
            }
        };
        safe_insert_neighbor(0, 1);
        safe_insert_neighbor(1, 2);
        safe_insert_neighbor(2, 0);
    }
    // std::set<O> asdf;
    std::list<std::list<FixedArray<float, 3>>> result;
    while(!neighbors.empty()) {
        std::list<FixedArray<float, 3>> contour;
        auto v0 = neighbors.begin()->first;
        auto v = v0;
        while(neighbors.find(v) != neighbors.end()) {
            contour.push_back(v);
            // assert(asdf.find(v) == asdf.end());
            // asdf.insert(v);
            auto old_v = v;
            v = neighbors.at(v);
            neighbors.erase(old_v);
        }
        // Get around comparison-operator ambiguity.
        const FixedArray<float, 3>& vv = v;
        const FixedArray<float, 3>& vv0 = v0;
        if (any(vv != vv0)) {
            // plot_mesh(ArrayShape{8000, 8000}, triangles, contour, {}).save_to_file("/tmp/cc.pgm");
            throw std::runtime_error("Contour is not closed");
        }
        neighbors.erase(v);
        result.push_back(contour);
    }
    return result;
}
