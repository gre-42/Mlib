#pragma once
#include <string>

namespace Mlib {

enum class ActorTask {
    UNDEFINED,
    AIR_CRUISE,
    GROUND_CRUISE,
    RUNWAY_ACCELERATE,
    RUNWAY_TAKEOFF,
    END
};

ActorTask actor_task_from_string(const std::string& str);
std::string actor_task_to_string(ActorTask actor_task);

}
