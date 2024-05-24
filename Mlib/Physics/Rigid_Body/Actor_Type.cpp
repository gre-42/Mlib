#include "Actor_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

ActorType Mlib::actor_type_from_string(const std::string& str) {
    static const std::map<std::string, ActorType> m{
        {"destination_reached_status", ActorType::DESTINATION_REACHED_STATUS},
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
    case ActorType::DESTINATION_REACHED_STATUS:
        return "destination_reached_status";
    case ActorType::TIRES:
        return "tires";
    case ActorType::WINGS:
        return "wings";
    case ActorType::END:
        ; // Fall through
    }
    THROW_OR_ABORT("Unknown actor type: " + std::to_string((int)actor_type));
}
