#include "Osm_Resource_Config.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box_Json.hpp>
#include <Mlib/Json/Chrono.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Waysides_Surface.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Waysides_Vertex.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>

using namespace Mlib;

namespace CoastArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(width);
}

void Mlib::from_json(const nlohmann::json& j, CoastConfiguration& water) {
    JsonView jv{ j };
    jv.validate(CoastArgs::options);
    water.width = jv.at<CompressedScenePos>(CoastArgs::width);
}

namespace WaterArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(texture);
DECLARE_ARGUMENT(animation_duration);
DECLARE_ARGUMENT(aabb);
DECLARE_ARGUMENT(cell_size);
DECLARE_ARGUMENT(duplicate_distance);
DECLARE_ARGUMENT(height);
DECLARE_ARGUMENT(coast);
DECLARE_ARGUMENT(holes_from_terrain);
}

void Mlib::from_json(const nlohmann::json& j, WaterConfiguration& water) {
    JsonView jv{ j };
    jv.validate(WaterArgs::options);
    water.texture = jv.at<VariableAndHash<std::string>>(WaterArgs::texture);
    water.animation_duration = jv.at<std::chrono::steady_clock::duration>(WaterArgs::animation_duration);
    water.aabb = jv.at<DefaultUnitialized<AxisAlignedBoundingBox<CompressedScenePos, 2>>>(WaterArgs::aabb);
    water.cell_size = jv.at<UFixedArray<CompressedScenePos, 2>>(WaterArgs::cell_size);
    water.duplicate_distance = jv.at<CompressedScenePos>(WaterArgs::duplicate_distance);
    water.height = jv.at<CompressedScenePos>(WaterArgs::height);
    water.coast = jv.at<CoastConfiguration>(WaterArgs::coast);
    water.holes_from_terrain = jv.at<bool>(WaterArgs::holes_from_terrain);
}

OsmResourceConfig::OsmResourceConfig()
{}

OsmResourceConfig::~OsmResourceConfig()
{}
