#pragma once
#include <map>
#include <string>

namespace Mlib {

struct Node;
struct Way;

void report_osm_problems(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
