#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <map>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
class TriangleList;
struct ColoredVertex;

class Rectangle {
public:
    /**
     * Create rectangle for line segment (b .. c), with given widths,
     * contained in crossings [aL; aR] >-- (b -- c) --< [dL; dR].
     */
    static bool from_line(
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
        float width_cdR);

    void draw_z0(
        TriangleList& tl,
        std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
        const std::string& b,
        const std::string& c,
        const FixedArray<float, 3>& color = {1.f, 1.f, 1.f},
        float uv0_x = 0,
        float uv1_x = 1,
        float uv0_y = 0,
        float uv1_y = 1,
        float start = -1,
        float stop = 1,
        bool rotate_texture = false,
        bool with_b_height_binding = false,
        bool with_c_height_binding = false) const;
    
    void draw(
        TriangleList& tl,
        std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
        const std::string& b,
        const std::string& c,
        const std::vector<FixedArray<ColoredVertex, 3>>& triangles,
        float scale,
        float width,
        float height) const;

    void draw_z(TriangleList& tl, float z0, float z1, const FixedArray<float, 3>& color = {1.f, 1.f, 1.f });

    FixedArray<float, 2> p00_;
    FixedArray<float, 2> p01_;
    FixedArray<float, 2> p10_;
    FixedArray<float, 2> p11_;
};

class WarpedSegment {
public:
    explicit WarpedSegment(const Rectangle& r);
    FixedArray<float, 2> warp_0(float x) const;
    FixedArray<float, 2> warp_1(float x) const;
    FixedArray<float, 3> warp_0(const FixedArray<float, 3>& p, float scale, float width, float height) const;
    FixedArray<float, 3> warp_1(const FixedArray<float, 3>& p, float scale, float width, float height) const;
private:
    FixedArray<float, 2> c0_;
    FixedArray<float, 2> c1_;
    FixedArray<float, 2> d0_;
    FixedArray<float, 2> d1_;
};

struct CurbedStreet {
    explicit CurbedStreet(const Rectangle& r, float start, float stop);
    FixedArray<float, 2> s00;
    FixedArray<float, 2> s10;
    FixedArray<float, 2> s01;
    FixedArray<float, 2> s11;
};

}
