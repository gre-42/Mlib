#pragma once
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture.hpp>
#include <list>
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
    float area = 0;
    std::string style;
};

}
