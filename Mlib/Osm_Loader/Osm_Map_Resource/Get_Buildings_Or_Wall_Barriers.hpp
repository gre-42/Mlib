#pragma once
#include <list>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

struct Building;
struct Way;
class FacadeTextureCycle;
struct SocleTexture;
enum class VerticalSubdivision;

enum class BuildingType {
    BUILDING,
    WALL_BARRIER,
    SPAWN_LINE
};

std::list<Building> get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Way>& ways,
    float building_bottom,
    float default_building_top,
    bool default_snap_height,
    float uv_scale_facade,
    const std::vector<SocleTexture>& socle_textures,
    FacadeTextureCycle& entrance_ftc,
    FacadeTextureCycle& middle_ftc,
    VerticalSubdivision default_vertical_subdivision);

}
