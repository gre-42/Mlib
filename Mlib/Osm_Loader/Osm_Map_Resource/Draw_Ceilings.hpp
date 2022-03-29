#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class TriangleList;
struct Node;
struct Building;
struct OsmResourceConfig;

void draw_ceilings(
    std::list<std::shared_ptr<TriangleList>>& tls_buildings,
    const OsmResourceConfig& config,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes);

}
