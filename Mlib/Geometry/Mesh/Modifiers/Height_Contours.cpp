#include "Height_Contours.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <map>

using namespace Mlib;

using Edge = std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>>;

static Edge swap(const Edge& e) {
    return Edge{e.second, e.first};
}

FixedArray<CompressedScenePos, 2> intersection_point(
    const Edge& edge,
    CompressedScenePos height)
{
    const auto& a = edge.first;
    const auto& b = edge.second;
    auto a2 = FixedArray<CompressedScenePos, 2>{a(0), a(1)};
    auto b2 = FixedArray<CompressedScenePos, 2>{b(0), b(1)};
    auto dh = funpack(b(2) - a(2));
    if (std::abs(dh) < 1e-12) {
        return { a(0), a(1) };
    } else if (dh > 0) {
        return lerp(a2, b2, funpack(height - a(2)) / dh);
    } else {
        return lerp(b2, a2, funpack(height - b(2)) / (-dh));
    }
}

std::list<std::list<FixedArray<CompressedScenePos, 2>>> Mlib::height_contours_by_edge(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    CompressedScenePos height)
{
    std::map<Edge, Edge> edge_neighbors;
    for (const auto& cva : cvas) {
        for (const auto& t : cva->triangles) {
            // linfo() << "triangle";
            for (size_t i = 0; i < 3; ++i) {
                Edge es[] = {
                    Edge{
                        t(i).position,
                        t((i + 1) % 3).position
                    },
                    Edge{
                        t((i + 1) % 3).position,
                        t((i + 2) % 3).position
                    }
                };
                const auto& center = t((i + 1) % 3).position;
                if (![&](){
                    for (auto& e : es) {
                        {
                            auto mi = std::min(e.first(2), e.second(2));
                            auto ma = std::max(e.first(2), e.second(2));
                            if ((mi > height) || (ma < height)) {
                                return false;
                            }
                        }
                        if ((e.first(2) == height) && (e.second(2) == height)) {
                            // Do nothing
                        } else if (e.first(2) == height) {
                            e.second = e.first;
                        } else if (e.second(2) == height) {
                            e.first = e.second;
                        }
                    }
                    return true;
                }()){
                    continue;
                }
                // linfo() << h << " - " << a << " - " << b;
                auto add_neighbor = [&edge_neighbors](
                    const Edge& ne0,
                    const Edge& ne1)
                {
                    // linfo() << "Add neighbor " << v0 << " -> " << v1;
                    auto res = edge_neighbors.try_emplace(ne0, ne1);
                    if (!res.second && (res.first->second != ne1)) {
                        // THROW_OR_ABORT2((EdgeException{v0, v1, "Vertex has multiple neighbors"}));
                        THROW_OR_ABORT2((EdgeException{ne0.first, ne0.second, "Edge has multiple neighbors"}));
                    }
                };
                if (es[0] != es[1]) {
                    if (center(2) < height) {
                        add_neighbor(es[1], swap(es[0]));
                    } else {
                        add_neighbor(es[0], swap(es[1]));
                    }
                }
            }
        }
    }
    // while (std::erase_if(edge_neighbors, [&](const auto& item){
    //         const auto& [key, value] = item;
    //         return !edge_neighbors.contains(value);
    //     }) != 0);
    auto ecs = find_neighbor_contours<Edge>(
        edge_neighbors,
        [&](const Edge& e){ return intersection_point(e, height); });
    std::list<std::list<FixedArray<CompressedScenePos, 2>>> pcs;
    for (const auto& ec : ecs) {
        auto& c = pcs.emplace_back();
        std::optional<FixedArray<CompressedScenePos, 2>> p0;
        for (const auto& e : ec) {
            auto p1 = intersection_point(e, height);
            if (p0.has_value()) {
                if (sum(squared(p1 - *p0)) > 1e-12) {
                    c.push_back(p1);
                }
            } else {
                c.push_back(p1);
            }
            p0 = p1;
        }
    }
    return pcs;
}

std::list<std::list<FixedArray<CompressedScenePos, 2>>> Mlib::height_contours_by_vertex(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    CompressedScenePos height)
{
    auto h = funpack(height);
    std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 2>> neighbors;
    for (const auto& cva : cvas) {
        for (const auto& t : cva->triangles) {
            // linfo() << "triangle";
            auto n = triangle_normal(funpack(FixedArray<CompressedScenePos, 3, 3>{
                t(0).position,
                t(1).position,
                t(2).position}));
            auto ln = FixedArray<ScenePos, 2>{n(1), -n(0)};
            // linfo() << "n " << n;
            std::optional<FixedArray<CompressedScenePos, 2>> p0;
            for (size_t i = 0; i < 3; ++i) {
                const auto& a = t(i).position;
                const auto& b = t((i + 1) % 3).position;
                {
                    auto mi = funpack(std::min(a(2), b(2)));
                    auto ma = funpack(std::max(a(2), b(2)));
                    if ((mi > h) || (ma < h)) {
                        continue;
                    }
                }
                // linfo() << h << " - " << a << " - " << b;
                auto p1 = [&]() -> FixedArray<CompressedScenePos, 2> {
                    auto a2 = FixedArray<CompressedScenePos, 2>{a(0), a(1)};
                    auto b2 = FixedArray<CompressedScenePos, 2>{b(0), b(1)};
                    auto dh = funpack(b(2) - a(2));
                    if (std::abs(dh) < 1e-12) {
                        return { a(0), a(1) };
                    } else if (dh > 0) {
                        return lerp(a2, b2, funpack(height - a(2)) / dh);
                    } else {
                        return lerp(b2, a2, funpack(height - b(2)) / (-dh));
                    }
                }();
                auto add_neighbor = [&](
                    const FixedArray<CompressedScenePos, 2>& v0,
                    const FixedArray<CompressedScenePos, 2>& v1)
                {
                    // linfo() << "Add neighbor " << v0 << " -> " << v1;
                    auto res = neighbors.try_emplace(OrderableFixedArray<CompressedScenePos, 2>{v0}, v1);
                    if (!res.second && any(res.first->second != v1)) {
                        // THROW_OR_ABORT2((EdgeException{v0, v1, "Vertex has multiple neighbors"}));
                        THROW_OR_ABORT2((EdgeException{a, b, "Vertex has multiple neighbors"}));
                    }
                };
                auto add_edge = [&](
                    const FixedArray<CompressedScenePos, 2>& v0,
                    const FixedArray<CompressedScenePos, 2>& v1)
                {
                    auto d = v1 - v0;
                    auto dir = dot0d(ln, funpack(d));
                    // linfo() << "dir " << dir << " - " << ln << " - " << d << " - " << a << " - " << b;
                    if ((std::abs(dir) < 1e-12) || (dir < 0)) {
                        add_neighbor(v0, v1);
                    } else {
                        add_neighbor(v1, v0);
                    }
                };
                if (p0.has_value() && any(*p0 != p1)) {
                    add_edge(*p0, p1);
                }
                p0 = p1;
            }
        }
    }
    return find_neighbor_contours<FixedArray<CompressedScenePos, 2>>(neighbors);
}
