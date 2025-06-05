#pragma once
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Hash_Of_Pair.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <unordered_map>
#include <unordered_set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class ContourDetectionStrategy;

template <class TPos>
std::unordered_set<std::pair<OrderableFixedArray<TPos, 3>, OrderableFixedArray<TPos, 3>>>
    find_contour_edges(const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles);

template <class TPos>
std::unordered_set<std::pair<OrderableFixedArray<TPos, 3>, OrderableFixedArray<TPos, 3>>>
    find_contour_edges(const std::list<FixedArray<ColoredVertex<TPos>, 3>*>& triangles);

template <class TPos>
std::list<std::list<FixedArray<TPos, 3>>> find_contours(
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    ContourDetectionStrategy strategy);

template <class TPos>
std::list<std::list<FixedArray<TPos, 3>>> find_contours(
    const std::list<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    ContourDetectionStrategy strategy);

template <class TPoint, class TNeighborsMap>
std::list<std::list<TPoint>> find_neighbor_contours(TNeighborsMap& neighbors)
{
    std::list<std::list<TPoint>> result;
    while(!neighbors.empty()) {
        std::list<TPoint> contour;
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
        const TPoint& vv = v;
        const TPoint& vv0 = v0;
        if (any(vv != vv0)) {
            // plot_mesh(ArrayShape{8000, 8000}, triangles, contour, {}).save_to_file("/tmp/cc.pgm");
            // plot_mesh_svg("/tmp/cc.svg", 800, 800, triangles, contour, {});
            THROW_OR_ABORT2((EdgeException{vv, vv0, "Contour is not closed"}));
        }
        neighbors.erase(v);
        result.push_back(contour);
    }
    return result;
}

static const auto make_orderable_default = []<class T>(const T& v) {
    return v;
};

static const auto make_orderable_array = []<class T>(const T& v) {
    return OrderableFixedArray(v);
};

template <class TPoint, class TTriangle, class TMakeOrderable = decltype(make_orderable_default)>
void extract_triangles_inside_contours(
    const std::vector<std::vector<TPoint>>& contours,
    std::list<TTriangle>& triangles,
    std::vector<std::list<TTriangle>>& inner_triangles,
    const TMakeOrderable& make_orderable = make_orderable_default)
{
    using O = std::remove_cvref_t<decltype(make_orderable(*(TPoint*)nullptr))>;

    inner_triangles.resize(contours.size());

    std::unordered_map<std::pair<O, O>, size_t> contour_edges;
    std::unordered_map<std::pair<O, O>, size_t> inner_edges;
    // Convert contour pathes to edges, asserting that the contours are closed.
    for (size_t contour_id = 0; contour_id < contours.size(); ++contour_id) {
        const auto& contour = contours[contour_id];
        for (auto it = contour.begin(); it != contour.end(); ++it) {
            auto s = it;
            ++s;
            if ((s == contour.end()) &&
                (make_orderable(*it) == make_orderable(contour.front())))
            {
                THROW_OR_ABORT("delete_triangles_outside_contour: Contour is closed");
            }
            if (!contour_edges.insert(
                std::make_pair(
                    std::make_pair(O{*it}, O{s == contour.end() ? contour.front() : *s}),
                    contour_id)).second)
            {
                THROW_OR_ABORT("Could not insert contour edge");
            }
        }
    }
    // Delete inside triangles having a contour edge,
    // and add their inner edges to the "inner_edges" set.
    triangles.remove_if([&contour_edges, &inner_edges, &inner_triangles](const TTriangle& t){
        for (size_t i = 0; i < 3; ++i) {
            const auto& a = t[i];
            const auto& b = t[(i + 1) % 3];
            const auto& c = t[(i + 2) % 3];
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
    // Add all edges overlapping an inside edge
    // to the "inner_edges" set,
    // and loop until nothing changes.
    while(true) {
        size_t old_size = inner_edges.size();
        triangles.remove_if([&inner_edges, &inner_triangles, &contour_edges, &triangles, &contours](const TTriangle& t){
            size_t contour_id = SIZE_MAX;
            for (size_t i = 0; i < 3; ++i) {
                const auto& a = t[i];
                const auto& b = t[(i + 1) % 3];
                auto ba = std::make_pair(O{b}, O{a});
                auto it = inner_edges.find(ba);
                if (it != inner_edges.end()) {
                    if (contour_id == SIZE_MAX) {
                        contour_id = it->second;
                    } else {
                        if (contour_id != it->second) {
                            if (true) {
                                const auto& c = t[(i + 2) % 3];
                                auto debug_filename = try_getenv("CONTOUR_DEBUG_FILENAME");
                                if (debug_filename.has_value()) {
                                    const auto& dbf = *debug_filename;
                                    if (dbf.ends_with(".png")) {
                                        plot_mesh(
                                            FixedArray<size_t, 2>{ 8000u, 8000u },
                                            1,                                  // thickness
                                            4,                                  // point size
                                            triangles,
                                            { contours[contour_id], contours[it->second] },
                                            { O{a}, O{b}, O{c} },
                                            {}).T().reversed(0).save_to_file(dbf);
                                    } else if (dbf.ends_with(".svg")) {
                                        plot_mesh_svg(
                                            dbf,
                                            600.,
                                            600.,
                                            triangles,
                                            {},
                                            { contours[contour_id], contours[it->second] },
                                            { O{a}, O{b}, O{c} });
                                    } else {
                                        THROW_OR_ABORT("Unknown file extension: " + dbf);
                                    }
                                    throw edge_exception(a, b, "Could not determine contour ID (" + std::to_string(contour_id) + " vs. " + std::to_string(it->second) + "), debug image saved");
                                } else {
                                    throw edge_exception(a, b, "Could not determine contour ID (" + std::to_string(contour_id) + " vs. " + std::to_string(it->second) + "), consider setting the CONTOUR_DEBUG_FILENAME environment variable");
                                }
                            } else {
                                lerr() << "Could not determine contour ID";
                                return false;
                            }
                        }
                    }
                }
            }
            if (contour_id != SIZE_MAX) {
                for (size_t i = 0; i < 3; ++i) {
                    const auto& a = t[i];
                    const auto& b = t[(i + 1) % 3];
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
