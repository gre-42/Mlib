#include "Height_Contours.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <map>

using namespace Mlib;

std::list<std::list<FixedArray<CompressedScenePos, 2>>> Mlib::height_contours(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    CompressedScenePos height)
{
    auto h = funpack(height);
    std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 2>> neighbors;
    auto add_neighbor = [&neighbors](
        const FixedArray<CompressedScenePos, 2>& v0,
        const FixedArray<CompressedScenePos, 2>& v1)
    {
        // linfo() << "Add neighbor " << v0 << " -> " << v1;
        auto res = neighbors.try_emplace(OrderableFixedArray<CompressedScenePos, 2>{v0}, v1);
        if (!res.second && any(res.first->second != v1)) {
            THROW_OR_ABORT2((EdgeException{v0, v1, "Vertex has multiple neighbors"}));
        }
    };
    for (const auto& cva : cvas) {
        for (const auto& t : cva->triangles) {
            // linfo() << "triangle";
            auto n = triangle_normal(funpack(FixedArray<CompressedScenePos, 3, 3>{
                t(0).position,
                t(1).position,
                t(2).position}));
            auto ln = FixedArray<ScenePos, 2>{n(1), -n(0)};
            auto add_edge = [&](
                const FixedArray<CompressedScenePos, 2>& v0,
                const FixedArray<CompressedScenePos, 2>& v1)
            {
                auto d = v1 - v0;
                auto dir = dot0d(ln, funpack(d));
                if ((std::abs(dir) < 1e-12) || (dir < 0)) {
                    add_neighbor(v0, v1);
                } else {
                    add_neighbor(v1, v0);
                }
            };
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
                if (p0.has_value()) {
                    add_edge(*p0, p1);
                }
                p0 = p1;
            }
        }
    }
    return find_neighbor_contours<FixedArray<CompressedScenePos, 2>>(neighbors);
}
