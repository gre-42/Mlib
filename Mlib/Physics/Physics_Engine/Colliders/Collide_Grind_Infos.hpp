#pragma once
#include <list>
#include <memory>
#include <unordered_map>

namespace Mlib {

struct PhysicsEngineConfig;
class IContactInfo;
class RigidBodyVehicle;
struct GrindInfo;
struct StaticWorld;
void collide_grind_infos(
    const PhysicsEngineConfig& cfg,
    const StaticWorld& world,
    std::list<std::unique_ptr<IContactInfo>>& contact_infos,
    const std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos);

}
