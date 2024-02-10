#pragma once
#include <compare>
#include <cstdint>
#include <map>

namespace Mlib {

class SmokeParticleGenerator;
class RigidBodyVehicle;
struct IntersectionScene;
enum class PhysicsMaterial: uint32_t;
struct SurfaceContactInfo;

class CommutativeMaterialPair {
public:
    CommutativeMaterialPair(
        PhysicsMaterial a,
        PhysicsMaterial b)
    : material0_{std::min(a, b)},
      material1_{std::max(a, b)}
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
    SurfaceContactInfo* get_contact_info(
        PhysicsMaterial material0,
        PhysicsMaterial material1);
    SurfaceContactInfo* get_contact_info(const IntersectionScene& c);

private:
    std::map<CommutativeMaterialPair, SurfaceContactInfo> surface_contact_infos_;
};

}
