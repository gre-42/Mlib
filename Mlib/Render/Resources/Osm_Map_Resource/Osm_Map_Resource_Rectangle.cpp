#include "Osm_Map_Resource_Rectangle.hpp"
#include <Mlib/Geometry/Mesh/Lines_To_Rectangles.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Math/Math.hpp>

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
    TriangleList& tl,
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    const std::string& b,
    const std::string& c,
    const FixedArray<float, 3>& color,
    float uv0_x,
    float uv1_x,
    float uv0_y,
    float uv1_y,
    float start,
    float stop,
    bool rotate_texture,
    bool with_b_height_binding,
    bool with_c_height_binding) const
{
    CurbedStreet cs{*this, start, stop};

    if (with_b_height_binding) {
        height_bindings[OrderableFixedArray{cs.s00}].insert(b);
        height_bindings[OrderableFixedArray{cs.s01}].insert(b);
    }
    if (with_c_height_binding) {
        height_bindings[OrderableFixedArray{cs.s10}].insert(c);
        height_bindings[OrderableFixedArray{cs.s11}].insert(c);
    }

    tl.draw_rectangle_wo_normals(
        FixedArray<float, 3>{cs.s00(0), cs.s00(1), 0.f},
        FixedArray<float, 3>{cs.s01(0), cs.s01(1), 0.f},
        FixedArray<float, 3>{cs.s11(0), cs.s11(1), 0.f},
        FixedArray<float, 3>{cs.s10(0), cs.s10(1), 0.f},
        color,
        color,
        color,
        color,
        rotate_texture ? FixedArray<float, 2>{uv1_x, uv1_y} : FixedArray<float, 2>{uv0_x, uv0_y},
        rotate_texture ? FixedArray<float, 2>{uv0_x, uv1_y} : FixedArray<float, 2>{uv1_x, uv0_y},
        rotate_texture ? FixedArray<float, 2>{uv0_x, uv0_y} : FixedArray<float, 2>{uv1_x, uv1_y},
        rotate_texture ? FixedArray<float, 2>{uv1_x, uv0_y} : FixedArray<float, 2>{uv0_x, uv1_y});
}

void Rectangle::draw(
    TriangleList& tl,
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
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
            } else if (t(i).position(1) == 1) {
                p(i) = ws.warp_1(t(i).position, scale, width, height);
            } else {
                std::stringstream sstr;
                sstr << "Position.y not -1 or +1: " << t(i).position;
                throw std::runtime_error(sstr.str());
            }
            height_bindings[OrderableFixedArray<float, 2>{p(i)(0), p(i)(1)}].insert(b);
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
