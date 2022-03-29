#pragma once
#include <string>
#include <vector>

namespace Mlib {

struct ParsedResourceName;

struct WaysideResourceNames {
    float min_dist;
    float max_dist;
    std::vector<ParsedResourceName> resource_names;
};

}
