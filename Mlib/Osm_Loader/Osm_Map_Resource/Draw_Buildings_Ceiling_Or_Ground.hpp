#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class TriangleList;
struct Material;
struct Morphology;
struct Node;
struct Building;
enum class DrawBuildingPartType;

void draw_buildings_ceiling_or_ground(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    const Material& material,
    const Morphology& morphology,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float triangulation_scale,
    float uv_scale,
    float uv_period,
    float max_width,
    DrawBuildingPartType tpe);

}
