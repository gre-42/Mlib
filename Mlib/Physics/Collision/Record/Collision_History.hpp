#pragma once
#include <list>
#include <memory>
#include <unordered_map>

namespace Mlib {

struct PhysicsEngineConfig;
class RigidBodies;
class SatTracker;
class ContactSmokeGenerator;
struct Beacon;
class ContactInfo;
template <typename TData, size_t... tshape>
class FixedArray;
struct IntersectionSceneAndContact;
class RigidBodyVehicle;
struct GrindInfo;
class BaseLog;

struct CollisionHistory {
    bool burn_in;
    const PhysicsEngineConfig& cfg;
    const SatTracker& st;
    ContactSmokeGenerator& scdb;
    std::list<Beacon>* beacons;
    std::list<std::unique_ptr<ContactInfo>>& contact_infos;
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections;
    std::unordered_map<RigidBodyVehicle*, std::list<IntersectionSceneAndContact>>& concave_t0_intersections;
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos;
    std::unordered_map<RigidBodyVehicle*, std::list<FixedArray<double, 3>>>& ridge_intersection_points;
    BaseLog* base_log;
};

}
