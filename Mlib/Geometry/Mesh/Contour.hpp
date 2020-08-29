#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <map>
#include <set>

namespace Mlib {

std::list<std::list<FixedArray<float, 3>>> find_contours(
    const std::list<FixedArray<ColoredVertex, 3>>& triangles);

template <class TPoint, class TTriangle>
void delete_triangles_outside_contour(
    std::vector<TPoint>& contour,
    std::list<TTriangle>& triangles)
{
    using O = TPoint;

    std::set<std::pair<O, O>> contour_edges;
    for(auto it = contour.begin(); it != contour.end(); ++it) {
        auto s = it;
        ++s;
        if ((s == contour.end()) && (*it == contour.front())) {
            throw std::runtime_error("delete_triangles_outside_contour: Contour is closed");
        }
        contour_edges.insert(std::make_pair(
            O{*it},
            O{s == contour.end() ? contour.front() : *s}));
    }
    std::set<O> contour_vertices{contour.begin(), contour.end()};
    std::set<O> outer_vertices;
    for(auto it = triangles.begin(); it != triangles.end(); ) {
        auto s = it++;
        bool is_outside = false;
        for(size_t i = 0; i < 3; ++i) {
            auto& t = *s;
            if (contour_edges.find(std::make_pair(O{t(i)}, O{t((i + 1) % 3)})) != contour_edges.end()) {
                outer_vertices.insert(O{t((i + 2) % 3)});
                is_outside = true;
            }
        }
        if (is_outside) {
            triangles.erase(s);
        }
    }
    while(true) {
        size_t old_size = outer_vertices.size();
        for(auto& t : triangles) {
            for(size_t iv = 0; iv < 3; ++iv) {
                if (outer_vertices.find(O{t(iv)}) != outer_vertices.end()) {
                    for(size_t iv1 = 0; iv1 < 3; ++iv1) {
                        if (contour_vertices.find(O{t(iv1)}) == contour_vertices.end()) {
                            outer_vertices.insert(O{t(iv1)});
                        }
                    }
                }
            }
        }
        if (outer_vertices.size() == old_size) {
            break;
        }
    }
    for(auto it = triangles.begin(); it != triangles.end(); ) {
        auto s = it++;
        if (outer_vertices.find(O{(*s)(0)}) != outer_vertices.end() ||
            outer_vertices.find(O{(*s)(1)}) != outer_vertices.end() ||
            outer_vertices.find(O{(*s)(2)}) != outer_vertices.end())
        {
            triangles.erase(s);
        }
    }
}

}
