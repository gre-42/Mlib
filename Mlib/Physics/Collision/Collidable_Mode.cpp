#include "Collidable_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CollidableMode Mlib::collidable_mode_from_string(const std::string& mode) {
    if (mode == "terrain") {
        return CollidableMode::TERRAIN;
    } else if (mode == "small_static") {
        return CollidableMode::SMALL_STATIC;
    } if (mode == "small_moving") {
        return CollidableMode::SMALL_MOVING;
    }
    THROW_OR_ABORT("Unknown collidable mode: " + mode);
}
