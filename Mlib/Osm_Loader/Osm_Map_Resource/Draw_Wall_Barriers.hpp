#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

struct Node;
struct Building;
struct Material;
struct Morphology;
struct SteinerPointInfo;
template <class TPos>
class TriangleList;
struct BarrierStyle;

void draw_wall_barriers(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    const Material& material,
    const Morphology& morphology,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::map<std::string, BarrierStyle>& barrier_styles);

}
