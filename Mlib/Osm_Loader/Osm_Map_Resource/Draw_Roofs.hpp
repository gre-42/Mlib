#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

struct Material;
template <class TPos>
class TriangleList;
template <typename TData, size_t... tshape>
class FixedArray;
struct Building;
struct Node;

void draw_roofs(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    const Material& material,
    const FixedArray<float, 3>& color,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_length);

}
