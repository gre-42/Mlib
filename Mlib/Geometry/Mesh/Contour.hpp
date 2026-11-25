#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Mesh/Compute_Area.hpp>
#include <Mlib/Geometry/Mesh/Contour_Assignment_Conflict_Behavior.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Hashing/Hash_Of_Pair.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Os/Env.hpp>
#include <list>
#include <stdexcept>
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

template <class TPoint, class TNeighborsMap, class TToPoint>
std::list<std::list<TPoint>> find_neighbor_contours(
    TNeighborsMap& neighbors,
    const TToPoint& to_point)
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
        if (v != v0) {
            // plot_mesh(ArrayShape{8000, 8000}, triangles, contour, {}).save_to_file("/tmp/cc.pgm");
            // plot_mesh_svg("/tmp/cc.svg", 800, 800, triangles, contour, {});
            throw EdgeException{to_point(v), to_point(v0), "Contour is not closed"};
        }
        neighbors.erase(v);
        result.push_back(contour);
    }
    return result;
}

template <class TPoint, class TNeighborsMap>
std::list<std::list<TPoint>> find_neighbor_contours(TNeighborsMap& neighbors) {
    return find_neighbor_contours<TPoint, TNeighborsMap>(
        neighbors,
        [](const TPoint&p) -> const TPoint& { return p; });
}

static const auto make_orderable_default = []<class T>(const T& v) {
    return v;
};

static const auto make_orderable_array = []<class T>(const T& v) {
    return make_orderable(v);
};

template <class TPoint, class TTriangle, class TMakeOrderable = decltype(make_orderable_default)>
void extract_triangles_inside_contours(
    const std::vector<std::vector<TPoint>>& contours,
    std::list<TTriangle>& triangles,
    std::vector<std::list<TTriangle>>& inner_triangles,
    const TMakeOrderable& make_orderable = make_orderable_default,
    ContourAssignmentConflictBehavior contour_assignment_conflict_behavior = ContourAssignmentConflictBehavior::THROW)
{
    using O = std::remove_cvref_t<decltype(make_orderable(*(TPoint*)nullptr))>;

    inner_triangles.resize(contours.size());

    std::unordered_set<std::pair<O, O>> covered_contour_edges;
    std::unordered_map<std::pair<O, O>, size_t> contour_edges;
    std::unordered_map<std::pair<O, O>, size_t> inner_edges;
    // Convert contour pathes to edges, asserting that the contours are closed.
    for (size_t contour_id = 0; contour_id < contours.size(); ++contour_id) {
        const auto& contour = contours[contour_id];
        if (compute_area_ccw(contour, 1.) < 0.) {
            throw std::runtime_error("Contour is not counterclockwise");
        }
        for (auto it = contour.begin(); it != contour.end(); ++it) {
            auto s = it;
            ++s;
            if ((s == contour.end()) &&
                (make_orderable(*it) == make_orderable(contour.front())))
            {
                throw std::runtime_error("delete_triangles_outside_contour: Contour is closed");
            }
            auto a = O{*it};
            auto b = O{s == contour.end() ? contour.front() : *s};
            if (!contour_edges.insert(std::make_pair(std::make_pair(a, b), contour_id)).second)
            {
                throw edge_exception(a, b, "Could not insert contour edge");
            }
        }
    }
    std::list<TTriangle> removed_triangles;
    auto remove_triangle_if = [&](const std::function<bool(const TTriangle& t)>& predicate) {
        for (auto it = triangles.begin(); it != triangles.end();) {
            if (predicate(*it)) {
                removed_triangles.splice(removed_triangles.end(), triangles, it++);
            } else {
                ++it;
            }
        }
    };
    // Delete inside triangles having a contour edge,
    // and add their inner edges to the "inner_edges" set.
    remove_triangle_if([&](const TTriangle& t){
        for (size_t i = 0; i < 3; ++i) {
            const auto& a = t[i];
            const auto& b = t[(i + 1) % 3];
            const auto& c = t[(i + 2) % 3];
            auto ab = std::make_pair(O{a}, O{b});
            auto it = contour_edges.find(ab);
            if (it != contour_edges.end()) {
                if (!covered_contour_edges.insert(ab).second) {
                    throw edge_exception(a, b, "Could not insert contour edge");
                }
                if (auto bc = std::make_pair(O{b}, O{c}); !contour_edges.contains(bc)) {
                    auto bcr = inner_edges.insert(std::make_pair(bc, it->second));
                    if (!bcr.second && (bcr.first->second != it->second)) {
                        throw edge_exception(b, c, std::format("Conflicting contour IDs: {} vs. {}", it->second, bcr.first->second));
                    }
                }
                if (auto ca = std::make_pair(O{c}, O{a}); !contour_edges.contains(ca)) {
                    auto car = inner_edges.insert(std::make_pair(ca, it->second));
                    if (!car.second && (car.first->second != it->second)) {
                        throw edge_exception(c, a, std::format("Conflicting contour IDs: {} vs. {}", it->second, car.first->second));
                    }
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
        remove_triangle_if([&](const TTriangle& t){
            size_t contour_id = SIZE_MAX;
            for (size_t i = 0; i < 3; ++i) {
                const auto& a = t[i];
                const auto& b = t[(i + 1) % 3];
                auto ba = std::make_pair(O{b}, O{a});
                auto it = inner_edges.find(ba);
                if (it == inner_edges.end()) {
                    continue;
                }
                if (contour_id == SIZE_MAX) {
                    contour_id = it->second;
                } else {
                    if (contour_id != it->second) {
                        auto maybe_save_debug_image_and_throw = [&][[noreturn]](){
                            const auto& c = t[(i + 2) % 3];
                            auto debug_filename = try_getenv("CONTOUR_DEBUG_FILENAME");
                            if (debug_filename.has_value()) {
                                const auto& dbf = *debug_filename;
                                triangles.splice(triangles.end(), std::move(removed_triangles));
                                if (dbf.ends_with(".png")) {
                                    plot_mesh(
                                        FixedArray<size_t, 2>{ 12000u, 12000u },
                                        0,                                  // thickness
                                        2,                                  // point size
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
                                    throw std::runtime_error("Unknown file extension: " + dbf);
                                }
                                throw edge_exception(a, b,
                                    std::format("Could not determine contour ID ({} vs. {}), debug image saved", contour_id, it->second));
                            } else {
                                throw edge_exception(a, b,
                                    std::format(
                                        "Could not determine contour ID ({} vs. {}), "
                                        "consider setting the CONTOUR_DEBUG_FILENAME environment variable", contour_id, it->second));
                            }
                        };
                        switch (contour_assignment_conflict_behavior) {
                            case ContourAssignmentConflictBehavior::THROW:
                                maybe_save_debug_image_and_throw();
                            case ContourAssignmentConflictBehavior::WARN:
                                lwarn() << std::format("Could not determine contour ID ({} vs. {})", contour_id, it->second);
                                return true;
                            case ContourAssignmentConflictBehavior::IGNORE:
                                return true;
                        }
                        throw std::runtime_error("Unknown contour assignment conflict behavior");
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
