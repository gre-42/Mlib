#pragma once
#include <Mlib/Physics/Containers/Ridge_Map.hpp>
#include <Mlib/Scene_Precision.hpp>
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
template <class TPosition>
struct CollisionRidgeSphere;
struct StaticWorld;
class SurfaceContactDb;
struct PhysicsPhase;

struct CollisionHistory {
    const PhysicsEngineConfig& cfg;
    const PhysicsPhase& phase;
    const StaticWorld& world;
    const SatTracker& st;
    const SurfaceContactDb& surface_contact_db;
    ContactSmokeGenerator& csg;
    ITrailRenderer& tr;
    std::list<Beacon>* beacons;
    std::list<std::unique_ptr<IContactInfo>>& contact_infos;
    std::unordered_map<OrderableFixedArray<CompressedScenePos, 2, 3>, IntersectionSceneAndContact>& raycast_intersections;
    std::unordered_map<RigidBodyVehicle*, std::list<IntersectionSceneAndContact>>& concave_t0_intersections;
    std::unordered_map<RigidBodyVehicle*, GrindInfo>& grind_infos;
    std::unordered_map<RigidBodyVehicle*, std::list<FixedArray<ScenePos, 3>>>& ridge_intersection_points;
    RidgeMap& ridge_map;
    BaseLog* base_log;
};

}
