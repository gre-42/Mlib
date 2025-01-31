#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace Mlib {

struct Node;
struct Building;
struct Material;
struct Morphology;
struct SteinerPointInfo;
template <class TPos>
class TriangleList;
struct BarrierStyle;
template <class TPos>
class VertexHeightBinding;
template <typename TData, size_t... tshape>
class FixedArray;

void draw_wall_barriers(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    std::unordered_map<FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>>& vertex_height_bindings,
    const Material& material,
    const Morphology& morphology,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::map<std::string, BarrierStyle>& barrier_styles);

}
