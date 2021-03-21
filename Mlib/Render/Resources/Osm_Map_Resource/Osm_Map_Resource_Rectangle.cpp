#include "Osm_Map_Resource_Rectangle.hpp"
#include <Mlib/Geometry/Mesh/Lines_To_Rectangles.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Height_Binding.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Road_Type.hpp>

using namespace Mlib;

/**
 * Create rectangle for line segment (b .. c), with given widths,
 * contained in crossings [aL; ...; aR] >-- (b -- c) --< [dL; ...; dR].
 */
bool Rectangle::from_line(
    Rectangle& rect,
    const FixedArray<float, 2>& aL,
    const FixedArray<float, 2>& aR,
    const FixedArray<float, 2>& b,
    const FixedArray<float, 2>& c,
    const FixedArray<float, 2>& dL,
    const FixedArray<float, 2>& dR,
    float width_aLb,
    float width_aRb,
    float width_bc,
    float width_cdL,
    float width_cdR)
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
        width_bc,
        width_cdL,
        width_cdR);
}

void Rectangle::draw_z0(
    TriangleList& tl_road,
    TriangleList* tl_entrance,
    std::map<OrderableFixedArray<float, 2>, HeightBinding>& height_bindings,
    std::map<EntranceType, std::set<OrderableFixedArray<float, 2>>>& entrances,
    const std::string& b,
    const std::string& c,
    const FixedArray<float, 3>& color,
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
        height_bindings[OrderableFixedArray{cs.s00}] = b;
        height_bindings[OrderableFixedArray{cs.s01}] = b;
    }
    if (with_c_height_binding) {
        height_bindings[OrderableFixedArray{cs.s10}] = c;
        height_bindings[OrderableFixedArray{cs.s11}] = c;
    }
    if (b_entrance_type != EntranceType::NONE) {
        entrances[b_entrance_type].insert(OrderableFixedArray{cs.s00});
        entrances[b_entrance_type].insert(OrderableFixedArray{cs.s01});
    }
    if (c_entrance_type != EntranceType::NONE) {
        entrances[c_entrance_type].insert(OrderableFixedArray{cs.s10});
        entrances[c_entrance_type].insert(OrderableFixedArray{cs.s11});
    }

    typedef FixedArray<float, 2> V2;
    auto swp = [road_type](const FixedArray<float, 2>&uv ) {
        return road_type == RoadType::WALL
            ? V2{uv(1), uv(0)}
            : uv;
    };
    tl_road.draw_rectangle_wo_normals(
        FixedArray<float, 3>{cs.s00(0), cs.s00(1), 0.f},
        FixedArray<float, 3>{cs.s01(0), cs.s01(1), 0.f},
        FixedArray<float, 3>{cs.s11(0), cs.s11(1), 0.f},
        FixedArray<float, 3>{cs.s10(0), cs.s10(1), 0.f},
        /* b_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ color,
        /* b_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ color,
        /* c_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ color,
        /* c_entrance_type != EntranceType::NONE ? FixedArray<float, 3>{1.f, 0.f, 0.f} :*/ color,
        swp(orientation >= RectangleOrientation::CENTER ? V2{uv1_x, uv1_y} : V2{uv0_x, uv0_y}),
        swp(orientation >= RectangleOrientation::CENTER ? V2{uv0_x, uv1_y} : V2{uv1_x, uv0_y}),
        swp(orientation >= RectangleOrientation::CENTER ? V2{uv0_x, uv0_y} : V2{uv1_x, uv1_y}),
        swp(orientation >= RectangleOrientation::CENTER ? V2{uv1_x, uv0_y} : V2{uv0_x, uv1_y}));

    if (b_entrance_type != EntranceType::NONE && c_entrance_type != EntranceType::NONE) {
        throw std::runtime_error("Detected duplicate entrance types");
    }
    if (tl_entrance != nullptr) {
        if ((b_entrance_type == EntranceType::TUNNEL) ||
            (c_entrance_type == EntranceType::TUNNEL))
        {
            tl_entrance->draw_rectangle_wo_normals(
                FixedArray<float, 3>{cs.s00(0), cs.s00(1), 0.f},
                FixedArray<float, 3>{cs.s01(0), cs.s01(1), 0.f},
                FixedArray<float, 3>{cs.s11(0), cs.s11(1), 0.f},
                FixedArray<float, 3>{cs.s10(0), cs.s10(1), 0.f});
        } else if (c_entrance_type == EntranceType::BRIDGE)
        {
            if (orientation == RectangleOrientation::RIGHT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<float, 3>{p10_(0), p10_(1), 0.f},
                    FixedArray<float, 3>{cs.s00(0), cs.s00(1), 0.f},
                    FixedArray<float, 3>{cs.s01(0), cs.s01(1), 0.f});
            } else if (orientation == RectangleOrientation::LEFT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<float, 3>{p11_(0), p11_(1), 0.f},
                    FixedArray<float, 3>{cs.s00(0), cs.s00(1), 0.f},
                    FixedArray<float, 3>{cs.s01(0), cs.s01(1), 0.f});
            } else {
                tl_entrance->draw_rectangle_wo_normals(
                    FixedArray<float, 3>{cs.s00(0), cs.s00(1), 0.f},
                    FixedArray<float, 3>{cs.s01(0), cs.s01(1), 0.f},
                    FixedArray<float, 3>{p11_(0), p11_(1), 0.f},
                    FixedArray<float, 3>{p10_(0), p10_(1), 0.f});
            }
        } else if (b_entrance_type == EntranceType::BRIDGE)
        {
            if (orientation == RectangleOrientation::LEFT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<float, 3>{cs.s10(0), cs.s10(1), 0.f},
                    FixedArray<float, 3>{p01_(0), p01_(1), 0.f},
                    FixedArray<float, 3>{cs.s11(0), cs.s11(1), 0.f});
            } else if (orientation == RectangleOrientation::RIGHT) {
                tl_entrance->draw_triangle_wo_normals(
                    FixedArray<float, 3>{cs.s10(0), cs.s10(1), 0.f},
                    FixedArray<float, 3>{p00_(0), p00_(1), 0.f},
                    FixedArray<float, 3>{cs.s11(0), cs.s11(1), 0.f});
            } else {
                tl_entrance->draw_rectangle_wo_normals(
                    FixedArray<float, 3>{p00_(0), p00_(1), 0.f},
                    FixedArray<float, 3>{p01_(0), p01_(1), 0.f},
                    FixedArray<float, 3>{cs.s11(0), cs.s11(1), 0.f},
                    FixedArray<float, 3>{cs.s10(0), cs.s10(1), 0.f});
            }
        }
    }
}

void Rectangle::draw(
    TriangleList& tl,
    std::map<OrderableFixedArray<float, 2>, HeightBinding>& height_bindings,
    const std::string& b,
    const std::string& c,
    const std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    float scale,
    float width,
    float height) const
{
    WarpedSegment ws{*this};

    for (const auto& t : triangles) {
        FixedArray<FixedArray<float, 3>, 3> p;
        for (size_t i = 0; i < 3; ++i) {
            if (t(i).position(1) == -1) {
                p(i) = ws.warp_0(t(i).position, scale, width, height);
                height_bindings[OrderableFixedArray<float, 2>{p(i)(0), p(i)(1)}] = b;
            } else if (t(i).position(1) == 1) {
                p(i) = ws.warp_1(t(i).position, scale, width, height);
                height_bindings[OrderableFixedArray<float, 2>{p(i)(0), p(i)(1)}] = c;
            } else {
                std::stringstream sstr;
                sstr << "Position.y not -1 or +1: " << t(i).position;
                throw std::runtime_error(sstr.str());
            }
        }
        tl.draw_triangle_wo_normals(
            p(0),
            p(1),
            p(2),
            t(0).color,
            t(1).color,
            t(2).color,
            t(0).uv,
            t(1).uv,
            t(2).uv);
    }
}

void Rectangle::draw_z(TriangleList& tl, float z0, float z1, const FixedArray<float, 3>& color) {
    tl.draw_rectangle_wo_normals(
        FixedArray<float, 3>{p00_(0), p00_(1), z0},
        FixedArray<float, 3>{p01_(0), p01_(1), z1},
        FixedArray<float, 3>{p11_(0), p11_(1), z1},
        FixedArray<float, 3>{p10_(0), p10_(1), z0},
        color,
        color,
        color,
        color);
}

WarpedSegment::WarpedSegment(const Rectangle& r)
: c0_{(r.p00_ + r.p01_) / 2.f},
  c1_{(r.p10_ + r.p11_) / 2.f},
  d0_{r.p01_ - r.p00_},
  d1_{r.p11_ - r.p10_}
{}

FixedArray<float, 2> WarpedSegment::warp_0(float x) const
{
    return c0_ + (x / 2) * d0_;
}

FixedArray<float, 2> WarpedSegment::warp_1(float x) const
{
    return c1_ + (x / 2) * d1_;
}

FixedArray<float, 3> WarpedSegment::warp_0(const FixedArray<float, 3>& p, float scale, float width, float height) const
{
    auto w = warp_0(width * p(0));
    return FixedArray<float, 3>(w(0), w(1), height * scale * p(2));
}

FixedArray<float, 3> WarpedSegment::warp_1(const FixedArray<float, 3>& p, float scale, float width, float height) const
{
    auto w = warp_1(width * p(0));
    return FixedArray<float, 3>(w(0), w(1), height * scale * p(2));
}

CurbedStreet::CurbedStreet(const Rectangle& r, float start, float stop) {
    WarpedSegment ws{r};
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
