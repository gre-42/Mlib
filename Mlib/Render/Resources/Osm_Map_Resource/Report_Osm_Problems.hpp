#pragma once
#include <map>
#include <string>

namespace Mlib {

struct Node;
struct Way;

void report_osm_problems(
    std::map<std::string, Node>& nodes,
    std::map<std::string, Way>& ways);

}
