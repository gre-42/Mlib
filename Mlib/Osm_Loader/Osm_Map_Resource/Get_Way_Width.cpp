#include "Get_Way_Width.hpp"
#include <Mlib/Map/Map.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

float Mlib::get_way_width(
    const Map<std::string, std::string>& tags,
    float default_street_width,
    float default_lane_width)
{
    return ((tags.find("lanes") != tags.end()) && (tags.find("width") == tags.end()))
        ? default_lane_width * (float)safe_stou(tags.at("lanes"))
        : parse_meters(tags, "width", default_street_width);
}
