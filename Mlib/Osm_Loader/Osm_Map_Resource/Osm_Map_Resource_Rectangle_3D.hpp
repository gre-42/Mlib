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

struct OsmRectangle3D {
    
    void draw(
        TriangleList<CompressedScenePos>& tl,
        const UUVector<FixedArray<ColoredVertex<float>, 3>>& triangles,
        float scale,
        float width,
        float height,
        float uv0_y,
        float uv1_y) const;

    FixedArray<double, 3> p00_;
    FixedArray<double, 3> p01_;
    FixedArray<double, 3> p10_;
    FixedArray<double, 3> p11_;
};

class WarpedSegment3D {
public:
    explicit WarpedSegment3D(const OsmRectangle3D& r);
    FixedArray<double, 3> warp_0(double x) const;
    FixedArray<double, 3> warp_1(double x) const;
    FixedArray<double, 3> warp_01(const FixedArray<double, 3>& p, double scale, double width, double height) const;
private:
    const OsmRectangle3D& r_;
};

}
