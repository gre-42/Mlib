#pragma once
#include <string>

namespace Mlib {

enum class CollidableMode {
    NONE,
    STATIC,
    MOVING
};

CollidableMode collidable_mode_from_string(const std::string& mode);

}
