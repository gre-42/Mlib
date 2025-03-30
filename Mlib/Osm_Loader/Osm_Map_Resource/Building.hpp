#pragma once
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture.hpp>
#include <list>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>

namespace Mlib {

struct Way;

enum class BuildingLevelType {
    TOP,
    SOCLE,
    ENTRANCES,
    MIDDLE
};

enum class BuildingDetailType {
    HIGH,
    LOW,
    COMBINED,
    UNDEFINED
};

struct BuildingLevel {
    float top;
    float bottom;
    float extra_width = 0;
    BuildingLevelType type;
    FacadeTextureDescriptor facade_texture_descriptor;
};

// From: https://wiki.openstreetmap.org/wiki/DE:OSM-4D/Roof_table#Subtype_9
struct Roof9_2 {
    float width;
    float height;
};

struct Building {
    std::string id;
    const Way& way;
    std::list<BuildingLevel> levels;
    std::optional<Roof9_2> roof_9_2;
    float area;
    std::string style;
};

void from_json(const nlohmann::json& j, Roof9_2& roof9_2);

}
