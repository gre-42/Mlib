#pragma once
#include <string>

namespace Mlib {

enum class CollidableMode {
    TERRAIN,
    SMALL_STATIC,
    SMALL_MOVING
};

CollidableMode collidable_mode_from_string(const std::string& mode);

}
