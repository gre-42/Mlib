#include "Collidable_Mode.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

CollidableMode Mlib::collidable_mode_from_string(const std::string& mode) {
    static std::map<std::string, CollidableMode> m{
        {"none", CollidableMode::NONE},
        {"collide", CollidableMode::COLLIDE},
        {"collide|move", CollidableMode::COLLIDE | CollidableMode::MOVE}
    };
    auto it = m.find(mode);
    if (it == m.end()) {
        throw std::runtime_error("Unknown collidable mode: " + mode);
    }
    return it->second;
}
