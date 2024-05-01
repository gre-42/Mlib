#pragma once
#include <map>
#include <set>
#include <string>

namespace Mlib {

struct NodesAndWays;

NodesAndWays smoothen_ways(
    const NodesAndWays& naws,
    const std::set<std::string>& included_highways,
    const std::set<std::string>& included_aeroways,
    float default_street_width,
    float default_lane_width,
    float scale,
    float max_length);

}
