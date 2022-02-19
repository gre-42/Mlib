#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <map>
#include <memory>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

class RigidBodyVehicle;
class TransformedMesh;
template <class TData, size_t n>
class PlaneNd;
struct PhysicsEngineConfig;
class SatTracker;
struct Beacon;
class ContactInfo;
class BaseLog;
struct GrindInfo;
enum class CollisionType;
enum class PhysicsMaterial;

struct IntersectionScene {
    RigidBodyVehicle& o0;
    RigidBodyVehicle& o1;
    const std::shared_ptr<TransformedMesh>& mesh0;
    const std::shared_ptr<TransformedMesh>& mesh1;
    const FixedArray<FixedArray<float, 3>, 2>& l1;
    const FixedArray<FixedArray<float, 3>, 3>& t0;
    const PlaneNd<float, 3>& p0;
    const PhysicsEngineConfig& cfg;
    const SatTracker& st;
    std::list<Beacon>* beacons;
    std::list<std::unique_ptr<ContactInfo>>& contact_infos;
    std::map<RigidBodyVehicle*, GrindInfo>& grind_infos;
    size_t tire_id1;
    PhysicsMaterial mesh0_material;
    bool l1_is_normal;
    CollisionType default_collision_type;
    BaseLog* base_log;
};

void handle_line_triangle_intersection(const IntersectionScene& i);

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
