#pragma once
#include <Mlib/Math/Interp_Fwd.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace Mlib {

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

void draw_building_walls(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    std::unordered_map<const FixedArray<double, 3>*, VertexHeightBinding<double>>& vertex_height_bindings,
    const Material& material,
    const Morphology& morphology,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    float socle_ambient_occlusion,
    const UUInterp<float, FixedArray<float, 3>>& height_colors);

}
