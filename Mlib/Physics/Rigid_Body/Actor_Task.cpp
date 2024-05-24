#include "Actor_Task.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

ActorTask Mlib::actor_task_from_string(const std::string& str) {
    static const std::map<std::string, ActorTask> m{
        {"undefined", ActorTask::UNDEFINED},
        {"air_cruise", ActorTask::AIR_CRUISE},
        {"ground_cruise", ActorTask::GROUND_CRUISE},
        {"runway_accelerate", ActorTask::RUNWAY_ACCELERATE},
        {"runway_takeoff", ActorTask::RUNWAY_TAKEOFF}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown actor task: \"" + str + '"');
    }
    return it->second;
}

std::string Mlib::actor_task_to_string(ActorTask actor_task) {
    switch (actor_task) {
    case ActorTask::UNDEFINED:
        return "undefined";
    case ActorTask::AIR_CRUISE:
        return "air_cruise";
    case ActorTask::GROUND_CRUISE:
        return "ground_cruise";
    case ActorTask::RUNWAY_ACCELERATE:
        return "runway_accelerate";
    case ActorTask::RUNWAY_TAKEOFF:
        return "runway_takeoff";
    case ActorTask::END:
        ; // Fall through
    }
    THROW_OR_ABORT("Unknown actor state: " + std::to_string((int)actor_task));
}
