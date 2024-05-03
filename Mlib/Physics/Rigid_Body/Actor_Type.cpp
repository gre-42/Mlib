#include "Actor_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ActorType Mlib::actor_type_from_string(const std::string& str) {
    static const std::map<std::string, ActorType> m{
        {"tires", ActorType::TIRES},
        {"wings", ActorType::WINGS}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown actor type: \"" + str + '"');
    }
    return it->second;
}

std::string Mlib::actor_type_to_string(ActorType actor_type) {
    switch (actor_type) {
    case ActorType::TIRES:
        return "tires";
    case ActorType::WINGS:
        return "wings";
    case ActorType::END:
        ; // Fall through
    }
    THROW_OR_ABORT("Unknown actor type: " + std::to_string((int)actor_type));
}
