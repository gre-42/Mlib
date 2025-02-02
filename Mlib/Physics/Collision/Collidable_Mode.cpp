#include "Collidable_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

CollidableMode Mlib::collidable_mode_from_string(const std::string& mode) {
    static std::map<std::string, CollidableMode> m{
        {"none", CollidableMode::NONE},
        {"static", CollidableMode::STATIC},
        {"moving", CollidableMode::MOVING}
    };
    auto it = m.find(mode);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown collidable mode: " + mode);
    }
    return it->second;
}
