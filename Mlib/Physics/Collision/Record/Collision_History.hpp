#pragma once
#include <chrono>
#include <list>
#include <map>
#include <memory>
#include <unordered_map>

namespace Mlib {

struct PhysicsEngineConfig;
class RigidBodies;
class SatTracker;
class ContactSmokeGenerator;
class ITrailRenderer;
struct Beacon;
class IContactInfo;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;
struct IntersectionSceneAndContact;
class RigidBodyVehicle;
struct GrindInfo;
class BaseLog;
struct CollisionRidgeSphere;

struct CollisionHistory {
    bool burn_in;
    const PhysicsEngineConfig& cfg;
    const SatTracker& st;
    ContactSmokeGenerator& csg;
    ITrailRenderer& tr;
    std::list<Beacon>* beacons;
    std::list<std::unique_ptr<IContactInfo>>& contact_infos;
    std::unordered_map<const FixedArray<FixedArray<double, 3>, 2>*, IntersectionSceneAndContact>& raycast_intersections;
    std::unordered_map<RigidBodyVehicle*, std::list<IntersectionSceneAndContact>>& concave_t0_intersections;
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos;
    std::unordered_map<RigidBodyVehicle*, std::list<FixedArray<double, 3>>>& ridge_intersection_points;
    const std::map<std::pair<OrderableFixedArray<double, 3>, OrderableFixedArray<double, 3>>, const CollisionRidgeSphere*>& ridge_map;
    BaseLog* base_log;
    std::chrono::steady_clock::time_point time;
};

}
