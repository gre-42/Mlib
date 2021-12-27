#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct WayPoints;
struct Way;

std::list<WayPoints> get_way_points(const std::map<std::string, Way>& ways);

}
