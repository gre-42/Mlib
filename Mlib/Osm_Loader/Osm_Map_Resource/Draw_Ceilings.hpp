#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class TriangleList;
struct Node;
struct Building;
struct OsmResourceConfig;

void draw_ceilings(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_buildings,
    const OsmResourceConfig& config,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes);

}
