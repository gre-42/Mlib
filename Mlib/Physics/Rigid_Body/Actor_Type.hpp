#pragma once
#include <string>

namespace Mlib {

enum class ActorType {
    DESTINATION_REACHED_STATUS,
    TIRES,
    WINGS,
    END
};

ActorType actor_type_from_string(const std::string& str);
std::string actor_type_to_string(ActorType actor_type);

}
