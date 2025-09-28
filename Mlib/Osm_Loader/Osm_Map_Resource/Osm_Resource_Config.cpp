#include "Osm_Resource_Config.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box_Json.hpp>
#include <Mlib/Json/Chrono_Duration.hpp>
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

namespace WaterTextureArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(color);
DECLARE_ARGUMENT(alpha);
}

void Mlib::from_json(const nlohmann::json& j, WaterTextureConfiguration& textures) {
    JsonView jv{ j };
    jv.validate(WaterTextureArgs::options);
    textures.color = jv.at<std::vector<VariableAndHash<std::string>>>(WaterTextureArgs::color);
    textures.alpha = jv.at<std::vector<VariableAndHash<std::string>>>(WaterTextureArgs::alpha);
}

namespace WaterArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(textures);
DECLARE_ARGUMENT(animation_duration);
DECLARE_ARGUMENT(aabb);
DECLARE_ARGUMENT(cell_size);
DECLARE_ARGUMENT(duplicate_distance);
DECLARE_ARGUMENT(heights);
DECLARE_ARGUMENT(coast);
DECLARE_ARGUMENT(generate_tiles);
DECLARE_ARGUMENT(holes_from_terrain);
DECLARE_ARGUMENT(yangle);
}

void Mlib::from_json(const nlohmann::json& j, WaterConfiguration& water) {
    JsonView jv{ j };
    jv.validate(WaterArgs::options);
    water.textures = jv.at<WaterTextureConfiguration>(WaterArgs::textures);
    water.animation_duration = jv.at<std::chrono::steady_clock::duration>(WaterArgs::animation_duration);
    water.aabb = jv.at<DefaultUnitialized<AxisAlignedBoundingBox<CompressedScenePos, 2>>>(WaterArgs::aabb);
    water.cell_size = jv.at<EFixedArray<CompressedScenePos, 2>>(WaterArgs::cell_size);
    water.duplicate_distance = jv.at<CompressedScenePos>(WaterArgs::duplicate_distance);
    water.heights = jv.at<EFixedArray<CompressedScenePos, 2>>(WaterArgs::heights);
    water.coast = jv.at<CoastConfiguration>(WaterArgs::coast);
    water.generate_tiles = jv.at<bool>(WaterArgs::generate_tiles);
    water.holes_from_terrain = jv.at<bool>(WaterArgs::holes_from_terrain);
    water.yangle = jv.at<float>(WaterArgs::yangle) * degrees;
}

OsmResourceConfig::OsmResourceConfig()
{}

OsmResourceConfig::~OsmResourceConfig()
{}
