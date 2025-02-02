#include "Compute_Building_Area.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>

using namespace Mlib;

void Mlib::compute_building_area(
    std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale)
{
    for (auto& b : buildings) {
        b.area = (float)compute_area_clockwise(b.way.nd, nodes, scale);
        // if (b.area < 0.f) {
        //     lerr() << "Negative building area: ID " << b.id << " area " << b.area;
        // }
    }
}
