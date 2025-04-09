#pragma once
#include <Mlib/Math/Interp_Fwd.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
template <typename TData, size_t... tshape>
class FixedArray;
struct SteinerPointInfo;
template <class TPos>
class VertexHeightBinding;
struct Node;
struct Material;
struct Morphology;
struct Building;
template <class TPos>
class TriangleList;
struct BarrierStyle;
class ColorCycle;

void draw_building_walls(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>& displacements,
    const Material& material,
    const Morphology& morphology,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    float snap_length_ratio,
    float snap_length_angle,
    float socle_ambient_occlusion,
    const UUInterp<float, FixedArray<float, 3>>& height_colors,
    ColorCycle& color_cycle);

}
