#pragma once
#include <list>
#include <memory>
#include <unordered_map>

namespace Mlib {

struct PhysicsEngineConfig;
class RigidBodies;
class SatTracker;
struct Beacon;
class ContactInfo;
template <typename TData, size_t... tshape>
class FixedArray;
struct IntersectionSceneAndContact;
class RigidBodyVehicle;
struct GrindInfo;
class BaseLog;

struct CollisionHistory {
    const PhysicsEngineConfig& cfg;
    const SatTracker& st;
    std::list<Beacon>* beacons;
    std::list<std::unique_ptr<ContactInfo>>& contact_infos;
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections;
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos;
    BaseLog* base_log;
};

}
