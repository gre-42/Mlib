#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct SteinerPointInfo;
class VertexHeightBinding;
struct Node;
struct Material;
struct Building;
class TriangleList;
class FacadeTextureCycle;
struct BarrierStyle;

void draw_building_walls(
    std::list<std::shared_ptr<TriangleList>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    std::map<const FixedArray<float, 3>*, VertexHeightBinding>& vertex_height_bindings,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::vector<std::string>& socle_textures,
    float socle_ambient_occlusion,
    FacadeTextureCycle& ftc);

}
