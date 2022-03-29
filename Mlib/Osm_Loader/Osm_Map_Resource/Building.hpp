#pragma once
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
};

struct Building {
    std::string id;
    const Way& way;
    std::list<BuildingLevel> levels;
    float area = 0;
    std::string style;
};

}
