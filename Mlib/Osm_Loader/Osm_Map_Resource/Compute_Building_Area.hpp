#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Building;
struct Node;

void compute_building_area(
    std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale);

}
