#pragma once
#include <string>

namespace Mlib {

template <class TKey, class TValue>
class Map;

float get_way_width(
    const Map<std::string, std::string>& tags,
    float default_street_width,
    float default_lane_width);

}
