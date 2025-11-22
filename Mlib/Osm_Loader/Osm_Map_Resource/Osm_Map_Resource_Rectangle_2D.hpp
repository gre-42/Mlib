#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
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
    OsmRectangle2D(Uninitialized);
    /**
     * Create rectangle for line segment (b .. c), with given widths,
     * contained in crossings [aL; aR] >-- (b -- c) --< [dL; dR].
     */
    static bool from_line(
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
        CompressedScenePos width_cdR);

    void draw_z0(
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
        RoadType road_type) const;

    void draw(
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
        float uv1_y) const;
    
    void draw_z(
        TriangleList<CompressedScenePos>& tl,
        CompressedScenePos z0,
        CompressedScenePos z1,
        const FixedArray<float, 3>& c00 = {1.f, 0.f, 0.f},
        const FixedArray<float, 3>& c10 = {0.f, 1.f, 0.f},
        const FixedArray<float, 3>& c11 = {0.f, 0.f, 1.f},
        const FixedArray<float, 3>& c01 = {0.f, 1.f, 1.f},
        const FixedArray<float, 2>& u00 = {0.f, 0.f},
        const FixedArray<float, 2>& u10 = {1.f, 0.f},
        const FixedArray<float, 2>& u11 = {1.f, 1.f},
        const FixedArray<float, 2>& u01 = {0.f, 1.f});

    FixedArray<CompressedScenePos, 2> p00_;
    FixedArray<CompressedScenePos, 2> p01_;
    FixedArray<CompressedScenePos, 2> p10_;
    FixedArray<CompressedScenePos, 2> p11_;
};

class WarpedSegment2D {
public:
    explicit WarpedSegment2D(const OsmRectangle2D& r);
    FixedArray<CompressedScenePos, 2> warp_0(double x) const;
    FixedArray<CompressedScenePos, 2> warp_1(double x) const;
    FixedArray<CompressedScenePos, 3> warp_0(const FixedArray<double, 3>& p, double scale, double width, CompressedScenePos height) const;
    FixedArray<CompressedScenePos, 3> warp_1(const FixedArray<double, 3>& p, double scale, double width, CompressedScenePos height) const;
    FixedArray<CompressedScenePos, 3> warp(const FixedArray<double, 3>& p, double scale, double width, CompressedScenePos height) const;
private:
    const OsmRectangle2D& r_;
};

struct CurbedStreet {
    explicit CurbedStreet(const OsmRectangle2D& r, ScenePos start, ScenePos stop);
    FixedArray<CompressedScenePos, 2, 2, 2> s;
};

}
