#pragma once
#include <list>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

struct Building;
struct Node;
struct Way;
class FacadeTextureCycle;
struct SocleTexture;
enum class VerticalSubdivision;
struct Roof9_2;

enum class BuildingType {
    BUILDING,
    WALL_BARRIER,
    SPAWN_LINE
};

std::list<Building> get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    double scale,
    double max_length,
    float building_bottom,
    float default_building_top,
    bool default_snap_height,
    float uv_scale_facade,
    float socle_height,
    const std::vector<SocleTexture>& socle_textures,
    float default_roof_9_2_max_building_height,
    const Roof9_2* default_roof_9_2,
    FacadeTextureCycle& entrance_ftc,
    FacadeTextureCycle& middle_ftc,
    VerticalSubdivision default_vertical_subdivision);

}
