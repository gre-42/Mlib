#include "Renderable_Osm_Map_Rectangle.hpp"
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
    float uv0,
    float uv1,
    float start,
    float stop,
    bool rotate_texture,
    bool with_b_height_binding,
    bool with_c_height_binding)
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
        FixedArray<float, 3>{cs.s00(0), cs.s00(1), 0},
        FixedArray<float, 3>{cs.s01(0), cs.s01(1), 0},
        FixedArray<float, 3>{cs.s11(0), cs.s11(1), 0},
        FixedArray<float, 3>{cs.s10(0), cs.s10(1), 0},
        color,
        color,
        color,
        color,
        rotate_texture ? FixedArray<float, 2>{1.f, uv1} : FixedArray<float, 2>{0.f, uv0},
        rotate_texture ? FixedArray<float, 2>{0.f, uv1} : FixedArray<float, 2>{1.f, uv0},
        rotate_texture ? FixedArray<float, 2>{0.f, uv0} : FixedArray<float, 2>{1.f, uv1},
        rotate_texture ? FixedArray<float, 2>{1.f, uv0} : FixedArray<float, 2>{0.f, uv1});
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

CurbedStreet::CurbedStreet(const Rectangle& r, float start, float stop) {
    FixedArray<float, 2> c0 = (r.p00_ + r.p01_) / 2.f;
    FixedArray<float, 2> c1 = (r.p10_ + r.p11_) / 2.f;
    s00 = c0 + (start / 2) * (r.p01_ - r.p00_);
    s10 = c1 + (start / 2) * (r.p11_ - r.p10_);
    s01 = c0 + (stop / 2) * (r.p01_ - r.p00_);
    s11 = c1 + (stop / 2) * (r.p11_ - r.p10_);
    if (start == -1) {
        s00 = r.p00_;
        s10 = r.p10_;
    }
    if (stop == 1) {
        s01 = r.p01_;
        s11 = r.p11_;
    }
}
