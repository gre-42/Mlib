#pragma once
#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <map>

namespace Mlib {

class SmokeParticleGenerator;
class RigidBodyVehicle;
enum class PhysicsMaterial: uint32_t;
struct SurfaceContactInfo;

class CommutativeMaterialPair {
public:
    CommutativeMaterialPair(
        PhysicsMaterial a,
        PhysicsMaterial b)
        : material0_{ std::min(a, b) }
        , material1_{ std::max(a, b) }
    {}
    std::strong_ordering operator<=>(const CommutativeMaterialPair &) const = default;

private:
    PhysicsMaterial material0_; 
    PhysicsMaterial material1_;
};

class SurfaceContactDb {
public:
    SurfaceContactDb();
    ~SurfaceContactDb();
    void store_contact_info(
        SurfaceContactInfo info,
        PhysicsMaterial material0,
        PhysicsMaterial material1);
    const SurfaceContactInfo* get_contact_info(
        PhysicsMaterial material0,
        PhysicsMaterial material1) const;
    const SurfaceContactInfo* get_contact_info(
        PhysicsMaterial material0,
        PhysicsMaterial material1,
        size_t tire_id1) const;

private:
    std::map<CommutativeMaterialPair, SurfaceContactInfo> surface_contact_infos_;
};

}
