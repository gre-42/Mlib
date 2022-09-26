#pragma once
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture.hpp>
#include <list>
#include <string>

namespace Mlib {

struct Way;

enum class BuildingLevelType {
    TOP,
    SOCLE,
    MIDDLE
};

struct BuildingLevel {
    float top;
    float bottom;
    float extra_width = 0;
    BuildingLevelType type;
    FacadeTextureDescriptor facade_texture_descriptor;
};

struct Building {
    std::string id;
    const Way& way;
    std::list<BuildingLevel> levels;
    float area = 0;
    std::string style;
};

}
