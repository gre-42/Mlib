#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>

#pragma GCC push_options
#pragma GCC optimize ("O3")

namespace Mlib {

class RigidBody;
class TransformedMesh;
template <class TData, size_t n>
class PlaneNd;
struct PhysicsEngineConfig;
class SatTracker;
struct Beacon;
struct ContactInfo;
class BaseLog;

struct IntersectionScene {
    const std::shared_ptr<RigidBody>& o0;
    const std::shared_ptr<RigidBody>& o1;
    const std::shared_ptr<TransformedMesh>& mesh0;
    const std::shared_ptr<TransformedMesh>& mesh1;
    const FixedArray<FixedArray<float, 3>, 2>& l1;
    const FixedArray<FixedArray<float, 3>, 3>& t0;
    const PlaneNd<float, 3>& p0;
    const PhysicsEngineConfig& cfg;
    const SatTracker& st;
    std::list<Beacon>* beacons;
    std::list<std::unique_ptr<ContactInfo>>& contact_infos;
    size_t tire_id;
    bool mesh0_two_sided;
    bool lines_are_normals;
    BaseLog* base_log;
};

void handle_line_triangle_intersection(const IntersectionScene& i);

}

#pragma GCC pop_options
