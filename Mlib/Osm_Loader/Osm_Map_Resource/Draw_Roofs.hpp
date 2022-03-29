#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

struct Material;
class TriangleList;
template <typename TData, size_t... tshape>
class FixedArray;
struct Building;
struct Node;

void draw_roofs(
    std::list<std::shared_ptr<TriangleList>>& tls,
    const Material& material,
    const FixedArray<float, 3>& color,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float width,
    float scale,
    float z0,
    float z1);

}
