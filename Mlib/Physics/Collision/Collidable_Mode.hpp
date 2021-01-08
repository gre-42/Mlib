#pragma once
#include <string>

namespace Mlib {

enum class CollidableMode {
    TERRAIN,
    SMALL_STATIC,
    SMALL_MOVING
};

inline CollidableMode collidable_mode_from_string(const std::string& mode) {
    if (mode == "terrain") {
        return CollidableMode::TERRAIN;
    } else if (mode == "small_static") {
        return CollidableMode::SMALL_STATIC;
    } if (mode == "small_moving") {
        return CollidableMode::SMALL_MOVING;
    }
    throw std::runtime_error("Unknown collidable mode: " + mode);
}

}
