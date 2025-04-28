#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way_Vertex.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct OsmRectangle2D;
struct Building;
struct Node;
enum class DrawBuildingPartType;
enum class BuildingDetailType;

struct BuildingVertex {
    SubdividedWayVertex orig;
    FixedArray<CompressedScenePos, 2> indented;
};

struct BuildingLevelOutline {
    std::list<BuildingVertex> outline;
    CompressedScenePos z;
};

struct BuildingSegment {
    FixedArray<SubdividedWayVertex, 2> orig;
    FixedArray<CompressedScenePos, 2, 2> indented;
};

std::list<BuildingSegment> smooth_building_level(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    double max_length,
    double width0,
    double width1,
    double scale);

BuildingLevelOutline smooth_building_level_outline(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    double scale,
    double max_length,
    DrawBuildingPartType tpe,
    BuildingDetailType detail);

std::list<std::list<BuildingSegment>> straight_building_level(
    const std::list<BuildingSegment>& level,
    float snap_length_angle);

}
