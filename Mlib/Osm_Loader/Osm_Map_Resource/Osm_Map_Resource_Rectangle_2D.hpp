#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <map>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
template <class TPos>
class TriangleList;
template <class TPos>
struct ColoredVertex;
enum class EntranceType;
class NodeHeightBinding;
enum class RoadType;

enum class RectangleOrientation {
    LEFT,
    CENTER,
    RIGHT
};

struct OsmRectangle2D {
    /**
     * Create rectangle for line segment (b .. c), with given widths,
     * contained in crossings [aL; aR] >-- (b -- c) --< [dL; dR].
     */
    static bool from_line(
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
        double width_cdR);

    void draw_z0(
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
        RoadType road_type) const;

    void draw(
        TriangleList<double>& tl,
        TriangleList<double>* tl_racing_line,
        float racing_line_uv0_sx,
        float racing_line_uv1_sx,
        float racing_line_uv0_dx,
        float racing_line_uv1_dx,
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
        float uv_sx,
        float uv0_y,
        float uv1_y) const;
    
    void draw_z(TriangleList<double>& tl, double z0, double z1, const FixedArray<float, 3>& color = {1.f, 1.f, 1.f });

    FixedArray<double, 2> p00_;
    FixedArray<double, 2> p01_;
    FixedArray<double, 2> p10_;
    FixedArray<double, 2> p11_;
};

class WarpedSegment2D {
public:
    explicit WarpedSegment2D(const OsmRectangle2D& r);
    FixedArray<double, 2> warp_0(double x) const;
    FixedArray<double, 2> warp_1(double x) const;
    FixedArray<double, 3> warp_0(const FixedArray<double, 3>& p, double scale, double width, double height) const;
    FixedArray<double, 3> warp_1(const FixedArray<double, 3>& p, double scale, double width, double height) const;
private:
    const OsmRectangle2D& r_;
};

struct CurbedStreet {
    explicit CurbedStreet(const OsmRectangle2D& r, double start, double stop);
    FixedArray<double, 2> s00;
    FixedArray<double, 2> s10;
    FixedArray<double, 2> s01;
    FixedArray<double, 2> s11;
};

}
