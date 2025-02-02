#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Node;

double compute_area_clockwise(
    const std::list<std::string>& nd,
    const std::map<std::string, Node>& nodes,
    double scale);

}
