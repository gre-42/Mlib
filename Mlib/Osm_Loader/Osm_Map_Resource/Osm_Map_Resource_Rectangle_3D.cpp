#include "Osm_Map_Resource_Rectangle_3D.hpp"
#include <Mlib/Geometry/Mesh/Lines_To_Rectangles.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Node_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void OsmRectangle3D::draw(
    TriangleList<CompressedScenePos>& tl,
    const UUVector<FixedArray<ColoredVertex<float>, 3>>& triangles,
    float scale,
    float width,
    float height,
    float uv0_y,
    float uv1_y) const
{
    WarpedSegment3D ws{*this};

    for (const auto& t : triangles) {
        FixedArray<FixedArray<double, 3>, 3> p = uninitialized;
        for (size_t i = 0; i < 3; ++i) {
            p(i) = ws.warp_01(t(i).position.casted<double>(), scale, width, height);
        }
        if (std::isnan(uv0_y) != std::isnan(uv1_y)) {
            THROW_OR_ABORT("Inconsistent UV NaN-ness");
        }
        {
            FixedArray<FixedArray<float, 2>, 3> uv = uninitialized;
            if (std::isnan(uv0_y)) {
                for (size_t i = 0; i < 3; ++i) {
                    uv(i) = t(i).uv;
                }
            } else {
                for (size_t i = 0; i < 3; ++i) {
                    uv(i)(0) = t(i).uv(0);
                    if (t(i).uv(1) < 0 || t(i).uv(1) > 1) {
                        std::stringstream sstr;
                        sstr << "uv.y not between 0 and 1: " << t(i).uv(1);
                        THROW_OR_ABORT(sstr.str());
                    }
                    uv(i)(1) = (1.f - t(i).uv(1)) * uv0_y + t(i).uv(1) * uv1_y;
                }
            }
            tl.draw_triangle_wo_normals(
                p(0).casted<CompressedScenePos>(),
                p(1).casted<CompressedScenePos>(),
                p(2).casted<CompressedScenePos>(),
                t(0).color,
                t(1).color,
                t(2).color,
                uv(0),
                uv(1),
                uv(2));
        }
    }
}

WarpedSegment3D::WarpedSegment3D(const OsmRectangle3D& r)
: r_{r}
{}

FixedArray<double, 3> WarpedSegment3D::warp_0(double x) const
{
    return ((1 - x) / 2) * r_.p00_ + ((x + 1) / 2) * r_.p01_;
}

FixedArray<double, 3> WarpedSegment3D::warp_1(double x) const
{
    return ((1 - x) / 2) * r_.p10_ + ((x + 1) / 2) * r_.p11_;
}

FixedArray<double, 3> WarpedSegment3D::warp_01(const FixedArray<double, 3>& p, double scale, double width, double height) const
{
    double x = p(1);
    if (std::abs(x) > 1) {
        std::stringstream sstr;
        sstr << "Position.y not between -1 and +1: " << x;
        THROW_OR_ABORT(sstr.str());
    }
    auto w0 = warp_0(width * p(0));
    auto w1 = warp_1(width * p(0));
    auto w = ((1 - x) / 2) * w0 + ((x + 1) / 2) * w1;
    return FixedArray<double, 3>(w(0), w(1), w(2) + height * scale * p(2));
}
