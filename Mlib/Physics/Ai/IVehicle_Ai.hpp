#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <cstddef>
#include <list>
#include <optional>
#include <vector>

namespace Mlib {

struct SkillFactor;
class SkillMap;
class AiWaypoint;
struct StaticWorld;

enum class VehicleAiMoveToStatus {
    NONE = 0,
    SCENE_VEHICLE_IS_NULL = (1 << 0),
    AUTOPILOT_IS_NULL = (1 << 1),
    SKILL_IS_MISSING = (1 << 2),
    WAYPOINT_IS_NAN = (1 << 3),
    POWER_IS_NAN = (1 << 4),
    WAYPOINT_REACHED = (1 << 5),
    RESTING_POSITION_REACHED = (1 << 6),
    STOPPED_TO_AVOID_COLLISION = (1 << 7),
    AUTOPILOT_SINGULARITY = (1 << 8)
};

inline VehicleAiMoveToStatus& operator |= (VehicleAiMoveToStatus& a, VehicleAiMoveToStatus b) {
    (int&)a |= (int)b;
    return a;
}

inline VehicleAiMoveToStatus operator | (VehicleAiMoveToStatus a, VehicleAiMoveToStatus b) {
    return (VehicleAiMoveToStatus)((int)a | (int)b);
}

inline VehicleAiMoveToStatus operator & (VehicleAiMoveToStatus a, VehicleAiMoveToStatus b) {
    return (VehicleAiMoveToStatus)((int)a & (int)b);
}

inline bool any(VehicleAiMoveToStatus a) {
    return a != VehicleAiMoveToStatus::NONE;
}

class IVehicleAi: public virtual DanglingBaseClass, public virtual DestructionNotifier {
public:
    virtual ~IVehicleAi() = default;
    virtual VehicleAiMoveToStatus move_to(
        const AiWaypoint& ai_waypoint,
        const SkillMap* skills,
        const StaticWorld& world,
        float dt) = 0;
    virtual std::vector<SkillFactor> skills() const = 0;
};

}
