#include "Osm_Map_Resource_Rectangle_2D.hpp"
#include <Mlib/Geometry/Mesh/Lines_To_Rectangles.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Node_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>

using namespace Mlib;

OsmRectangle2D::OsmRectangle2D(Uninitialized)
    : p00_{ uninitialized }
    , p01_{ uninitialized }
    , p10_{ uninitialized }
    , p11_{ uninitialized }
{}

/**
 * Create rectangle for line segment (b .. c), with given widths,
 * contained in crossings [aL; ...; aR] >-- (b -- c) --< [dL; ...; dR].
 */
bool OsmRectangle2D::from_line(
    OsmRectangle2D& rect,
    const FixedArray<CompressedScenePos, 2>& aL,
    const FixedArray<CompressedScenePos, 2>& aR,
    const FixedArray<CompressedScenePos, 2>& b,
    const FixedArray<CompressedScenePos, 2>& c,
    const FixedArray<CompressedScenePos, 2>& dL,
    const FixedArray<CompressedScenePos, 2>& dR,
    CompressedScenePos width_aLb,
    CompressedScenePos width_aRb,
    CompressedScenePos width_bcL,
    CompressedScenePos width_bcR,
    CompressedScenePos width_cdL,
    CompressedScenePos width_cdR)
{
    return lines_to_rectangles(
        rect.p00_,
        rect.p01_,
        rect.p10_,
        rect.p11_,
        aL,
        aR,
        b,
        c,
        dL,
        dR,
        width_aLb,
        width_aRb,
        width_bcL,
        width_bcR,
        width_cdL,
        width_cdR);
}

void OsmRectangle2D::draw_z0(
    TriangleList<CompressedScenePos>& tl_road,
    TriangleList<CompressedScenePos>* tl_racing_line,
    float uv0_sx,
    float uv1_sx,
    float uv0_dx,
    float uv1_dx,
    float racing_line_uv0_y,
    float racing_line_uv1_y,
    bool flip_racing_line,
    const FixedArray<float, 3>& racing_line_color0,
    const FixedArray<float, 3>& racing_line_color1,
    TriangleList<CompressedScenePos>* tl_entrance,
    std::map<OrderableFixedArray<CompressedScenePos, 2>, NodeHeightBinding>& node_height_bindings,
    std::map<EntranceType, std::set<OrderableFixedArray<CompressedScenePos, 2>>>& entrances,
    const std::string& b,
    const std::string& c,
    const FixedArray<float, 3>& color0,
    const FixedArray<float, 3>& color1,
    float uv0_x,
    float uv1_x,
    float uv0_y,
    float uv1_y,
    float start,
    float stop,
    RectangleOrientation orientation,
    bool with_b_height_binding,
    bool with_c_height_binding,
    EntranceType b_entrance_type,
    EntranceType c_entrance_type,
    RoadType road_type) const
{
    CurbedStreet cs{*this, start, stop};

    // Road:
    // b, 00 >-->-->-->-->--> c, 10
    // b, 01 >-->-->-->-->--> c, 11

    if (with_b_height_binding) {
        node_height_bindings[OrderableFixedArray{cs.s[0][0]}] = b;
        node_height_bindings[OrderableFixedArray{cs.s[0][1]}] = b;
    }
    if (with_c_height_binding) {
        node_height_bindings[OrderableFixedArray{cs.s[1][0]}] = c;
        node_height_bindings[OrderableFixedArray{cs.s[1][1]}] = c;
    }
    if (b_entrance_type != EntranceType::NONE) {
        entrances[b_entrance_type].insert(OrderableFixedArray{cs.s[0][0]});
        entrances[b_entrance_type].insert(OrderableFixedArray{cs.s[0][1]});
    }
    if (c_entrance_type != EntranceType::NONE) {
        entrances[c_entrance_type].insert(OrderableFixedArray{cs.s[1][0]});
        entrances[c_entrance_type].insert(OrderableFixedArray{cs.s[1][1]});
    }

    {
        using V2 = FixedArray<float, 2>;
        auto swp = [road_type](const FixedArray<float, 2>&uv) {
            return road_type == RoadType::WALL
                ? V2{uv(1), uv(0)}
                : uv;
        };
        tl_road.draw_rectangle_wo_normals(
            FixedArray<CompressedScenePos, 3>{cs.s(0, 0, 0), cs.s(0, 0, 1), (CompressedScenePos)0.},
            FixedArray<CompressedScenePos, 3>{cs.s(0, 1, 0), cs.s(0, 1, 1), (CompressedScenePos)0.},
            FixedArray<CompressedScenePos, 3>{cs.s(1, 1, 0), cs.s(1, 1, 1), (CompressedScenePos)0.},
            FixedArray<CompressedScenePos, 3>{cs.s(1, 0, 0), cs.s(1, 0, 1), (CompressedScenePos)0.},
            /* b_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ Colors::from_rgb(color0),
            /* b_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ Colors::from_rgb(color0),
            /* c_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ Colors::from_rgb(color1),
            /* c_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ Colors::from_rgb(color1),
            swp(orientation > RectangleOrientation::CENTER ? V2{uv1_x, uv1_y} : V2{uv0_x, uv0_y}),
            swp(orientation > RectangleOrientation::CENTER ? V2{uv0_x, uv1_y} : V2{uv1_x, uv0_y}),
            swp(orientation > RectangleOrientation::CENTER ? V2{uv0_x, uv0_y} : V2{uv1_x, uv1_y}),
            swp(orientation > RectangleOrientation::CENTER ? V2{uv1_x, uv0_y} : V2{uv0_x, uv1_y}),
            {},  // Bone weights
            {},  // Bone weights
            {},  // Bone weights
            {},  // Bone weights
            NormalVectorErrorBehavior::THROW,
            TriangleTangentErrorBehavior::THROW,
            RectangleTriangulationMode::DELAUNAY);
    }

    if (tl_racing_line != nullptr) {
        float rl_uv0_x = 0.f;
        float rl_uv1_x = 1.f;
        auto swp = [flip_racing_line](const FixedArray<float, 2>&uv) {
            return flip_racing_line
                ? 1.f - uv
                : uv;
        };
        tl_racing_line->draw_rectangle_wo_normals(
            FixedArray<CompressedScenePos, 3>{cs.s(0, 0, 0), cs.s(0, 0, 1), (CompressedScenePos)0.f},
            FixedArray<CompressedScenePos, 3>{cs.s(0, 1, 0), cs.s(0, 1, 1), (CompressedScenePos)0.f},
            FixedArray<CompressedScenePos, 3>{cs.s(1, 1, 0), cs.s(1, 1, 1), (CompressedScenePos)0.f},
            FixedArray<CompressedScenePos, 3>{cs.s(1, 0, 0), cs.s(1, 0, 1), (CompressedScenePos)0.f},
            Colors::from_rgb(racing_line_color0),
            Colors::from_rgb(racing_line_color0),
            Colors::from_rgb(racing_line_color1),
            Colors::from_rgb(racing_line_color1),
            swp(FixedArray<float, 2>{rl_uv0_x * uv0_sx + uv0_dx, racing_line_uv0_y}),
            swp(FixedArray<float, 2>{rl_uv1_x * uv1_sx + uv0_dx, racing_line_uv0_y}),
            swp(FixedArray<float, 2>{rl_uv1_x * uv1_sx + uv1_dx, racing_line_uv1_y}),
            swp(FixedArray<float, 2>{rl_uv0_x * uv0_sx + uv1_dx, racing_line_uv1_y}),
            {},  // bone weights
            {},  // bone weights
            {},  // bone weights
            {},  // bone weights
            NormalVectorErrorBehavior::THROW,
            TriangleTangentErrorBehavior::THROW,
            RectangleTriangulationMode::DELAUNAY);
    }
    if (b_entrance_type != EntranceType::NONE && c_entrance_type != EntranceType::NONE) {
        THROW_OR_ABORT("Detected duplicate entrance types");
    }
    if (tl_entrance != nullptr) {
        if ((b_entrance_type == EntranceType::TUNNEL) ||
            (c_entrance_type == EntranceType::TUNNEL))
        {
            tl_entrance->draw_rectangle_wo_normals(
                FixedArray<CompressedScenePos, 3>{cs.s(0, 0, 0), cs.s(0, 0, 1), (CompressedScenePos)0.},
                FixedArray<CompressedScenePos, 3>{cs.s(0, 1, 0), cs.s(0, 1, 1), (CompressedScenePos)0.},
                FixedArray<CompressedScenePos, 3>{cs.s(1, 1, 0), cs.s(1, 1, 1), (CompressedScenePos)0.},
                FixedArray<CompressedScenePos, 3>{cs.s(1, 0, 0), cs.s(1, 0, 1), (CompressedScenePos)0.},
                Colors::RED,    // c00
                Colors::GREEN,  // c10
                Colors::BLUE,   // c11
                Colors::CYAN,   // c01
                {0.f, 0.f},     // u00
                {1.f, 0.f},     // u10
                {1.f, 1.f},     // u11
                {0.f, 1.f},     // u01
                {},             // b00
                {},             // b10
                {},             // b11
                {},             // b01
                NormalVectorErrorBehavior::THROW,
                TriangleTangentErrorBehavior::THROW,
                RectangleTriangulationMode::DELAUNAY);
        } else if (c_entrance_type == EntranceType::BRIDGE)
        {
            if (orientation == RectangleOrientation::RIGHT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<CompressedScenePos, 3>{p10_(0), p10_(1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(0, 0, 0), cs.s(0, 0, 1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(0, 1, 0), cs.s(0, 1, 1), (CompressedScenePos)0.});
            } else if (orientation == RectangleOrientation::LEFT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<CompressedScenePos, 3>{p11_(0), p11_(1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(0, 0, 0), cs.s(0, 0, 1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(0, 1, 0), cs.s(0, 1, 1), (CompressedScenePos)0.});
            } else {
                tl_entrance->draw_rectangle_wo_normals(
                    FixedArray<CompressedScenePos, 3>{cs.s(0, 0, 0), cs.s(0, 0, 1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(0, 1, 0), cs.s(0, 1, 1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{p11_(0), p11_(1), (CompressedScenePos)0.f},
                    FixedArray<CompressedScenePos, 3>{p10_(0), p10_(1), (CompressedScenePos)0.f},
                    Colors::RED,    // c00
                    Colors::GREEN,  // c10
                    Colors::BLUE,   // c11
                    Colors::CYAN,   // c01
                    {0.f, 0.f},     // u00
                    {1.f, 0.f},     // u10
                    {1.f, 1.f},     // u11
                    {0.f, 1.f},     // u01
                    {},             // b00
                    {},             // b10
                    {},             // b11
                    {},             // b01
                    NormalVectorErrorBehavior::THROW,
                    TriangleTangentErrorBehavior::THROW,
                    RectangleTriangulationMode::DELAUNAY);
            }
        } else if (b_entrance_type == EntranceType::BRIDGE)
        {
            if (orientation == RectangleOrientation::LEFT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<CompressedScenePos, 3>{cs.s(1, 0, 0), cs.s(1, 0, 1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{p01_(0), p01_(1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(1, 1, 0), cs.s(1, 1, 1), (CompressedScenePos)0.});
            } else if (orientation == RectangleOrientation::RIGHT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<CompressedScenePos, 3>{cs.s(1, 0, 0), cs.s(1, 0, 1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{p00_(0), p00_(1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(1, 1, 0), cs.s(1, 1, 1), (CompressedScenePos)0.});
            } else {
                tl_entrance->draw_rectangle_wo_normals(
                    FixedArray<CompressedScenePos, 3>{p00_(0), p00_(1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{p01_(0), p01_(1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(1, 1, 0), cs.s(1, 1, 1), (CompressedScenePos)0.},
                    FixedArray<CompressedScenePos, 3>{cs.s(1, 0, 0), cs.s(1, 0, 1), (CompressedScenePos)0.},
                    Colors::RED,    // c00
                    Colors::GREEN,  // c10
                    Colors::BLUE,   // c11
                    Colors::CYAN,   // c01
                    {0.f, 0.f},     // u00
                    {1.f, 0.f},     // u10
                    {1.f, 1.f},     // u11
                    {0.f, 1.f},     // u01
                    {},             // b00
                    {},             // b10
                    {},             // b11
                    {},             // b01
                    NormalVectorErrorBehavior::THROW,
                    TriangleTangentErrorBehavior::THROW,
                    RectangleTriangulationMode::DELAUNAY);
            }
        }
    }
}

void OsmRectangle2D::draw(
    TriangleList<CompressedScenePos>& tl,
    TriangleList<CompressedScenePos>* tl_racing_line,
    float racing_line_uv0_sx,
    float racing_line_uv1_sx,
    float racing_line_uv0_dx,
    float racing_line_uv1_dx,
    float racing_line_uv0_y,
    float racing_line_uv1_y,
    bool flip_racing_line,
    const FixedArray<float, 3>& racing_line_color0,
    const FixedArray<float, 3>& racing_line_color1,
    std::map<OrderableFixedArray<CompressedScenePos, 2>, NodeHeightBinding>& node_height_bindings,
    const std::string& b,
    const std::string& c,
    const UUVector<FixedArray<ColoredVertex<float>, 3>>& triangles,
    float scale,
    float width,
    CompressedScenePos height,
    float uv_sx,
    float uv0_y,
    float uv1_y) const
{
    WarpedSegment2D ws{*this};

    for (const auto& t : triangles) {
        FixedArray<CompressedScenePos, 3, 3> p = uninitialized;
        for (size_t i = 0; i < 3; ++i) {
            float x = t(i).position(1);
            if (std::abs(x) > 1) {
                std::stringstream sstr;
                sstr << "Position.y not between -1 and +1: " << x;
                THROW_OR_ABORT(sstr.str());
            }
            auto a0 = ws.warp_0(t(i).position.casted<double>(), scale, width, height);
            auto a1 = ws.warp_1(t(i).position.casted<double>(), scale, width, height);
            p[i] = (((1. - x) / 2.) * funpack(a0) + ((x + 1.) / 2.) * funpack(a1)).casted<CompressedScenePos>();
            if (x < 0) {
                node_height_bindings[OrderableFixedArray<CompressedScenePos, 2>{p(i, 0), p(i, 1)}] = b;
            } else {
                node_height_bindings[OrderableFixedArray<CompressedScenePos, 2>{p(i, 0), p(i, 1)}] = c;
            }
            // if (t(i).position(1) == -1) {
            //     p(i) = ws.warp_0(t(i).position.casted<CompressedScenePos>(), scale, width, height);
            //     node_height_bindings[OrderableFixedArray<CompressedScenePos, 2>{p(i)(0), p(i)(1)}] = b;
            // } else if (t(i).position(1) == 1) {
            //     p(i) = ws.warp_1(t(i).position.casted<CompressedScenePos>(), scale, width, height);
            //     node_height_bindings[OrderableFixedArray<CompressedScenePos, 2>{p(i)(0), p(i)(1)}] = c;
            // } else {
            //     std::stringstream sstr;
            //     sstr << "Position.y not -1 or +1: " << t(i).position;
            //     THROW_OR_ABORT(sstr.str());
            // }
        }
        if (std::isnan(uv0_y) != std::isnan(uv1_y)) {
            THROW_OR_ABORT("Inconsistent UV NaN-ness");
        }
        {
            FixedArray<float, 3, 2> uv = uninitialized;
            if (std::isnan(uv0_y)) {
                for (size_t i = 0; i < 3; ++i) {
                    uv[i] = t(i).uv;
                }
            } else {
                for (size_t i = 0; i < 3; ++i) {
                    uv(i, 0) = t(i).uv(0);
                    float x = t(i).uv(1);
                    if ((x < 0) || (x > 1)) {
                        std::stringstream sstr;
                        sstr << "uv.y not between 0 and 1: " << x;
                        THROW_OR_ABORT(sstr.str());
                    }
                    uv(i, 1) = (1.f - x) * uv0_y + x * uv1_y;
                    // if (t(i).uv(1) == 0) {
                    //     uv(i)(1) = uv0_y;
                    // } else if (t(i).uv(1) == 1) {
                    //     uv(i)(1) = uv1_y;
                    // } else {
                    //     std::stringstream sstr;
                    //     sstr << "uv.y not 0 or 1: " << t(i).uv;
                    //     THROW_OR_ABORT(sstr.str());
                    // }
                }
            }
            for (size_t i = 0; i < 3; ++i) {
                uv(i, 0) *= uv_sx;
            }
            tl.draw_triangle_wo_normals(
                p[0],
                p[1],
                p[2],
                t(0).color,
                t(1).color,
                t(2).color,
                uv[0],
                uv[1],
                uv[2]);
        }
        if (tl_racing_line != nullptr) {
            if (std::isnan(racing_line_uv0_y) ||
                std::isnan(racing_line_uv1_y) ||
                std::isnan(racing_line_uv0_sx) ||
                std::isnan(racing_line_uv1_sx) ||
                std::isnan(racing_line_uv0_dx) ||
                std::isnan(racing_line_uv1_dx))
            {
                THROW_OR_ABORT("UV NaN despite racing line");
            }
            FixedArray<float, 3, 2> uv = uninitialized;
            FixedArray<float, 3, 3> color = uninitialized;
            for (size_t i = 0; i < 3; ++i) {
                float x = t(i).position(1);
                if (std::abs(x) > 1) {
                    std::stringstream sstr;
                    sstr << "Position.y not between -1 and +1: " << x;
                    THROW_OR_ABORT(sstr.str());
                }
                uv(i, 0) = float(
                    ((1. - x) / 2.) * (0.5 * (1. + t(i).position(0)) * racing_line_uv0_sx + racing_line_uv0_dx) +
                    ((x + 1.) / 2.) * (0.5 * (1. + t(i).position(0)) * racing_line_uv1_sx + racing_line_uv1_dx));
                uv(i, 1) = float((1. - x) / 2.) * racing_line_uv0_y + float((x + 1.) / 2.) * racing_line_uv1_y;
                color[i] = float((1. - x) / 2.) * racing_line_color0 +
                           float((x + 1.) / 2.) * racing_line_color1;
                // if (t(i).position(1) == -1) {
                //     uv(i)(0) = 0.5f * (1.f + t(i).position(0)) * racing_line_uv0_sx + racing_line_uv0_dx;
                //     uv(i)(1) = uv0_y;
                //     color(i) = racing_line_color0;
                // } else if (t(i).position(1) == 1) {
                //     uv(i)(0) = 0.5f * (1.f + t(i).position(0)) * racing_line_uv1_sx + racing_line_uv1_dx;
                //     uv(i)(1) = uv1_y;
                //     color(i) = racing_line_color1;
                // } else {
                //     std::stringstream sstr;
                //     sstr << "Position.y not -1 or 1: " << t(i).uv;
                //     THROW_OR_ABORT(sstr.str());
                // }
            }
            if (flip_racing_line) {
                uv = fixed_ones<float, 3, 2>() - uv;
            }
            tl_racing_line->draw_triangle_wo_normals(
                p[0],
                p[1],
                p[2],
                Colors::from_rgb(color[0]),
                Colors::from_rgb(color[1]),
                Colors::from_rgb(color[2]),
                uv[0],
                uv[1],
                uv[2]);
        }
    }
}

void OsmRectangle2D::draw_z(
    TriangleList<CompressedScenePos>& tl,
    CompressedScenePos z0,
    CompressedScenePos z1,
    const FixedArray<float, 3>& c00,
    const FixedArray<float, 3>& c10,
    const FixedArray<float, 3>& c11,
    const FixedArray<float, 3>& c01,
    const FixedArray<float, 2>& u00,
    const FixedArray<float, 2>& u10,
    const FixedArray<float, 2>& u11,
    const FixedArray<float, 2>& u01)
{
    tl.draw_rectangle_wo_normals(
        FixedArray<CompressedScenePos, 3>{p00_(0), p00_(1), z0},
        FixedArray<CompressedScenePos, 3>{p01_(0), p01_(1), z1},
        FixedArray<CompressedScenePos, 3>{p11_(0), p11_(1), z1},
        FixedArray<CompressedScenePos, 3>{p10_(0), p10_(1), z0},
        Colors::from_rgb(c00),
        Colors::from_rgb(c10),
        Colors::from_rgb(c11),
        Colors::from_rgb(c01),
        u00,
        u10,
        u11,
        u01);
}

WarpedSegment2D::WarpedSegment2D(const OsmRectangle2D& r)
: r_{r}
{}

FixedArray<CompressedScenePos, 2> WarpedSegment2D::warp_0(double x) const
{
    return (((1 - x) / 2) * funpack(r_.p00_) + ((x + 1) / 2) * funpack(r_.p01_)).casted<CompressedScenePos>();
}

FixedArray<CompressedScenePos, 2> WarpedSegment2D::warp_1(double x) const
{
    return (((1 - x) / 2) * funpack(r_.p10_) + ((x + 1) / 2) * funpack(r_.p11_)).casted<CompressedScenePos>();
}

FixedArray<CompressedScenePos, 3> WarpedSegment2D::warp_0(const FixedArray<double, 3>& p, double scale, double width, CompressedScenePos height) const
{
    auto w = warp_0(width * p(0));
    return FixedArray<CompressedScenePos, 3>(w(0), w(1), height * scale * p(2));
}

FixedArray<CompressedScenePos, 3> WarpedSegment2D::warp_1(const FixedArray<double, 3>& p, double scale, double width, CompressedScenePos height) const
{
    auto w = warp_1(width * p(0));
    return FixedArray<CompressedScenePos, 3>(w(0), w(1), height * scale * p(2));
}

CurbedStreet::CurbedStreet(const OsmRectangle2D& r, ScenePos start, ScenePos stop)
    : s{ uninitialized }
{
    WarpedSegment2D ws{r};
    s[0][0] = ws.warp_0(start);
    s[1][0] = ws.warp_1(start);
    s[0][1] = ws.warp_0(stop);
    s[1][1] = ws.warp_1(stop);
    if (start == -1) {
        s[0][0] = r.p00_;
        s[1][0] = r.p10_;
    }
    if (stop == 1) {
        s[0][1] = r.p01_;
        s[1][1] = r.p11_;
    }
}
