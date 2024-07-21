#pragma once
#include <map>
#include <string>

namespace Mlib {

template <class TBaseMap>
class GenericMap;

template <class TKey, class TValue>
using Map = GenericMap<std::map<TKey, TValue>>;

float get_way_width(
    const Map<std::string, std::string>& tags,
    float default_street_width,
    float default_lane_width);

}
