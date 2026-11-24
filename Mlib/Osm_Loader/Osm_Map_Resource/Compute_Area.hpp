#pragma once
#include <Mlib/Osm_Loader/Osm_Map_Resource/Elements.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

double compute_area_clockwise(
    const std::list<std::string>& nd,
    const std::map<std::string, Node>& nodes,
    double scale);
    
}
