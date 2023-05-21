#include "Collidable_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CollidableMode Mlib::collidable_mode_from_string(const std::string& mode) {
    if (mode == "static") {
        return CollidableMode::STATIC;
    } else if (mode == "moving") {
        return CollidableMode::MOVING;
    }
    THROW_OR_ABORT("Unknown collidable mode: " + mode);
}
