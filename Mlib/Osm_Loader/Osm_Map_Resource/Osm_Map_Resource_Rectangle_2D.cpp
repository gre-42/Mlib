#include "Osm_Map_Resource_Rectangle_2D.hpp"
#include <Mlib/Geometry/Mesh/Lines_To_Rectangles.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Node_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>

using namespace Mlib;

/**
 * Create rectangle for line segment (b .. c), with given widths,
 * contained in crossings [aL; ...; aR] >-- (b -- c) --< [dL; ...; dR].
 */
bool OsmRectangle2D::from_line(
    OsmRectangle2D& rect,
    const FixedArray<double, 2>& aL,
    const FixedArray<double, 2>& aR,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    const FixedArray<double, 2>& dL,
    const FixedArray<double, 2>& dR,
    double width_aLb,
    double width_aRb,
    double width_bcL,
    double width_bcR,
    double width_cdL,
    double width_cdR)
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
    TriangleList<double>& tl_road,
    TriangleList<double>* tl_racing_line,
    float uv0_sx,
    float uv1_sx,
    float uv0_dx,
    float uv1_dx,
    bool flip_racing_line,
    const FixedArray<float, 3>& racing_line_color0,
    const FixedArray<float, 3>& racing_line_color1,
    TriangleList<double>* tl_entrance,
    std::map<OrderableFixedArray<double, 2>, NodeHeightBinding>& node_height_bindings,
    std::map<EntranceType, std::set<OrderableFixedArray<double, 2>>>& entrances,
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
        node_height_bindings[OrderableFixedArray{cs.s00}] = b;
        node_height_bindings[OrderableFixedArray{cs.s01}] = b;
    }
    if (with_c_height_binding) {
        node_height_bindings[OrderableFixedArray{cs.s10}] = c;
        node_height_bindings[OrderableFixedArray{cs.s11}] = c;
    }
    if (b_entrance_type != EntranceType::NONE) {
        entrances[b_entrance_type].insert(OrderableFixedArray{cs.s00});
        entrances[b_entrance_type].insert(OrderableFixedArray{cs.s01});
    }
    if (c_entrance_type != EntranceType::NONE) {
        entrances[c_entrance_type].insert(OrderableFixedArray{cs.s10});
        entrances[c_entrance_type].insert(OrderableFixedArray{cs.s11});
    }

    {
        typedef FixedArray<float, 2> V2;
        auto swp = [road_type](const FixedArray<float, 2>&uv) {
            return road_type == RoadType::WALL
                ? V2{uv(1), uv(0)}
                : uv;
        };
        tl_road.draw_rectangle_wo_normals(
            FixedArray<double, 3>{cs.s00(0), cs.s00(1), 0.},
            FixedArray<double, 3>{cs.s01(0), cs.s01(1), 0.},
            FixedArray<double, 3>{cs.s11(0), cs.s11(1), 0.},
            FixedArray<double, 3>{cs.s10(0), cs.s10(1), 0.},
            /* b_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ color0,
            /* b_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ color0,
            /* c_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ color1,
            /* c_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ color1,
            swp(orientation > RectangleOrientation::CENTER ? V2{uv1_x, uv1_y} : V2{uv0_x, uv0_y}),
            swp(orientation > RectangleOrientation::CENTER ? V2{uv0_x, uv1_y} : V2{uv1_x, uv0_y}),
            swp(orientation > RectangleOrientation::CENTER ? V2{uv0_x, uv0_y} : V2{uv1_x, uv1_y}),
            swp(orientation > RectangleOrientation::CENTER ? V2{uv1_x, uv0_y} : V2{uv0_x, uv1_y}));
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
            FixedArray<double, 3>{cs.s00(0), cs.s00(1), 0.f},
            FixedArray<double, 3>{cs.s01(0), cs.s01(1), 0.f},
            FixedArray<double, 3>{cs.s11(0), cs.s11(1), 0.f},
            FixedArray<double, 3>{cs.s10(0), cs.s10(1), 0.f},
            racing_line_color0,
            racing_line_color0,
            racing_line_color1,
            racing_line_color1,
            swp(FixedArray<float, 2>{rl_uv0_x * uv0_sx + uv0_dx, uv0_y}),
            swp(FixedArray<float, 2>{rl_uv1_x * uv1_sx + uv0_dx, uv0_y}),
            swp(FixedArray<float, 2>{rl_uv1_x * uv1_sx + uv1_dx, uv1_y}),
            swp(FixedArray<float, 2>{rl_uv0_x * uv0_sx + uv1_dx, uv1_y}));
    }
    if (b_entrance_type != EntranceType::NONE && c_entrance_type != EntranceType::NONE) {
        throw std::runtime_error("Detected duplicate entrance types");
    }
    if (tl_entrance != nullptr) {
        if ((b_entrance_type == EntranceType::TUNNEL) ||
            (c_entrance_type == EntranceType::TUNNEL))
        {
            tl_entrance->draw_rectangle_wo_normals(
                FixedArray<double, 3>{cs.s00(0), cs.s00(1), 0.},
                FixedArray<double, 3>{cs.s01(0), cs.s01(1), 0.},
                FixedArray<double, 3>{cs.s11(0), cs.s11(1), 0.},
                FixedArray<double, 3>{cs.s10(0), cs.s10(1), 0.});
        } else if (c_entrance_type == EntranceType::BRIDGE)
        {
            if (orientation == RectangleOrientation::RIGHT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<double, 3>{p10_(0), p10_(1), 0.},
                    FixedArray<double, 3>{cs.s00(0), cs.s00(1), 0.},
                    FixedArray<double, 3>{cs.s01(0), cs.s01(1), 0.});
            } else if (orientation == RectangleOrientation::LEFT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<double, 3>{p11_(0), p11_(1), 0.},
                    FixedArray<double, 3>{cs.s00(0), cs.s00(1), 0.},
                    FixedArray<double, 3>{cs.s01(0), cs.s01(1), 0.});
            } else {
                tl_entrance->draw_rectangle_wo_normals(
                    FixedArray<double, 3>{cs.s00(0), cs.s00(1), 0.},
                    FixedArray<double, 3>{cs.s01(0), cs.s01(1), 0.},
                    FixedArray<double, 3>{p11_(0), p11_(1), 0.f},
                    FixedArray<double, 3>{p10_(0), p10_(1), 0.f});
            }
        } else if (b_entrance_type == EntranceType::BRIDGE)
        {
            if (orientation == RectangleOrientation::LEFT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<double, 3>{cs.s10(0), cs.s10(1), 0.},
                    FixedArray<double, 3>{p01_(0), p01_(1), 0.},
                    FixedArray<double, 3>{cs.s11(0), cs.s11(1), 0.});
            } else if (orientation == RectangleOrientation::RIGHT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<double, 3>{cs.s10(0), cs.s10(1), 0.},
                    FixedArray<double, 3>{p00_(0), p00_(1), 0.},
                    FixedArray<double, 3>{cs.s11(0), cs.s11(1), 0.});
            } else {
                tl_entrance->draw_rectangle_wo_normals(
                    FixedArray<double, 3>{p00_(0), p00_(1), 0.},
                    FixedArray<double, 3>{p01_(0), p01_(1), 0.},
                    FixedArray<double, 3>{cs.s11(0), cs.s11(1), 0.},
                    FixedArray<double, 3>{cs.s10(0), cs.s10(1), 0.});
            }
        }
    }
}

void OsmRectangle2D::draw(
    TriangleList<double>& tl,
    TriangleList<double>* tl_racing_line,
    float uv0_sx,
    float uv1_sx,
    float uv0_dx,
    float uv1_dx,
    bool flip_racing_line,
    const FixedArray<float, 3>& racing_line_color0,
    const FixedArray<float, 3>& racing_line_color1,
    std::map<OrderableFixedArray<double, 2>, NodeHeightBinding>& node_height_bindings,
    const std::string& b,
    const std::string& c,
    const std::vector<FixedArray<ColoredVertex<float>, 3>>& triangles,
    float scale,
    float width,
    float height,
    float uv0_y,
    float uv1_y) const
{
    WarpedSegment2D ws{*this};

    for (const auto& t : triangles) {
        FixedArray<FixedArray<double, 3>, 3> p;
        for (size_t i = 0; i < 3; ++i) {
            // double x = t(i).position(1);
            // if (std::abs(x) > 1) {
            //     std::stringstream sstr;
            //     sstr << "Position.y not between -1 and +1: " << x;
            //     throw std::runtime_error(sstr.str());
            // }
            // auto a0 = ws.warp_0(t(i).position.casted<double>(), scale, width, height);
            // auto a1 = ws.warp_1(t(i).position.casted<double>(), scale, width, height);
            // p(i) = ((1 - x) / 2) * a0 + ((x + 1) / 2) * a1;
            if (t(i).position(1) == -1) {
                p(i) = ws.warp_0(t(i).position.casted<double>(), scale, width, height);
                node_height_bindings[OrderableFixedArray<double, 2>{p(i)(0), p(i)(1)}] = b;
            } else if (t(i).position(1) == 1) {
                p(i) = ws.warp_1(t(i).position.casted<double>(), scale, width, height);
                node_height_bindings[OrderableFixedArray<double, 2>{p(i)(0), p(i)(1)}] = c;
            } else {
                std::stringstream sstr;
                sstr << "Position.y not -1 or +1: " << t(i).position;
                throw std::runtime_error(sstr.str());
            }
        }
        if (std::isnan(uv0_y) != std::isnan(uv1_y)) {
            throw std::runtime_error("Inconsistent UV NaN-ness");
        }
        {
            FixedArray<FixedArray<float, 2>, 3> uv;
            if (std::isnan(uv0_y)) {
                for (size_t i = 0; i < 3; ++i) {
                    uv(i) = t(i).uv;
                }
            } else {
                for (size_t i = 0; i < 3; ++i) {
                    uv(i)(0) = t(i).uv(0);
                    // uv(i)(1) = (1.f - t(i).uv(1)) * uv0_y + t(i).uv(1) * uv1_y;
                    if (t(i).uv(1) == 0) {
                        uv(i)(1) = uv0_y;
                    } else if (t(i).uv(1) == 1) {
                        uv(i)(1) = uv1_y;
                    } else {
                        std::stringstream sstr;
                        sstr << "uv.y not 0 or 1: " << t(i).uv;
                        throw std::runtime_error(sstr.str());
                    }
                }
            }
            tl.draw_triangle_wo_normals(
                p(0),
                p(1),
                p(2),
                t(0).color,
                t(1).color,
                t(2).color,
                uv(0),
                uv(1),
                uv(2));
        }
        if (tl_racing_line != nullptr) {
            if (std::isnan(uv0_y) ||
                std::isnan(uv1_y) ||
                std::isnan(uv0_sx) ||
                std::isnan(uv1_sx) ||
                std::isnan(uv0_dx) ||
                std::isnan(uv1_dx))
            {
                throw std::runtime_error("UV NaN despite racing line");
            }
            FixedArray<FixedArray<float, 2>, 3> uv;
            FixedArray<FixedArray<float, 3>, 3> color;
            for (size_t i = 0; i < 3; ++i) {
                if (t(i).position(1) == -1) {
                    uv(i)(0) = 0.5f * (1.f + t(i).position(0)) * uv0_sx + uv0_dx;
                    uv(i)(1) = uv0_y;
                    color(i) = racing_line_color0;
                } else if (t(i).position(1) == 1) {
                    uv(i)(0) = 0.5f * (1.f + t(i).position(0)) * uv1_sx + uv1_dx;
                    uv(i)(1) = uv1_y;
                    color(i) = racing_line_color1;
                } else {
                    std::stringstream sstr;
                    sstr << "position.y not -1 or 1: " << t(i).uv;
                    throw std::runtime_error(sstr.str());
                }
            }
            if (flip_racing_line) {
                uv = fixed_ones<float, 2>() - uv;
            }
            tl_racing_line->draw_triangle_wo_normals(
                p(0),
                p(1),
                p(2),
                color(0),
                color(1),
                color(2),
                uv(0),
                uv(1),
                uv(2));
        }
    }
}

void OsmRectangle2D::draw_z(TriangleList<double>& tl, double z0, double z1, const FixedArray<float, 3>& color) {
    tl.draw_rectangle_wo_normals(
        FixedArray<double, 3>{p00_(0), p00_(1), z0},
        FixedArray<double, 3>{p01_(0), p01_(1), z1},
        FixedArray<double, 3>{p11_(0), p11_(1), z1},
        FixedArray<double, 3>{p10_(0), p10_(1), z0},
        color,
        color,
        color,
        color);
}

WarpedSegment2D::WarpedSegment2D(const OsmRectangle2D& r)
: r_{r}
{}

FixedArray<double, 2> WarpedSegment2D::warp_0(double x) const
{
    return ((1 - x) / 2) * r_.p00_ + ((x + 1) / 2) * r_.p01_;
}

FixedArray<double, 2> WarpedSegment2D::warp_1(double x) const
{
    return ((1 - x) / 2) * r_.p10_ + ((x + 1) / 2) * r_.p11_;
}

FixedArray<double, 3> WarpedSegment2D::warp_0(const FixedArray<double, 3>& p, double scale, double width, double height) const
{
    auto w = warp_0(width * p(0));
    return FixedArray<double, 3>(w(0), w(1), height * scale * p(2));
}

FixedArray<double, 3> WarpedSegment2D::warp_1(const FixedArray<double, 3>& p, double scale, double width, double height) const
{
    auto w = warp_1(width * p(0));
    return FixedArray<double, 3>(w(0), w(1), height * scale * p(2));
}

CurbedStreet::CurbedStreet(const OsmRectangle2D& r, double start, double stop) {
    WarpedSegment2D ws{r};
    s00 = ws.warp_0(start);
    s10 = ws.warp_1(start);
    s01 = ws.warp_0(stop);
    s11 = ws.warp_1(stop);
    if (start == -1) {
        s00 = r.p00_;
        s10 = r.p10_;
    }
    if (stop == 1) {
        s01 = r.p01_;
        s11 = r.p11_;
    }
}
