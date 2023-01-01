#pragma once
#include <compare>
#include <cstddef>
#include <map>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SmokeParticleGenerator;
class RigidBodyVehicle;
struct IntersectionScene;
enum class PhysicsMaterial;
struct SurfaceContactInfo;

class UnorderedMaterialPair {
public:
    UnorderedMaterialPair(
        PhysicsMaterial a,
        PhysicsMaterial b)
    : material0_{std::min(a, b)},
      material1_{std::max(a, b)}
    {}
    std::strong_ordering operator <=> (const UnorderedMaterialPair&) const = default;
private:
    PhysicsMaterial material0_; 
    PhysicsMaterial material1_;
};

class SurfaceContactDb {
public:
    SurfaceContactDb();
    ~SurfaceContactDb();
    void store_contact_info(
        const SurfaceContactInfo& info,
        PhysicsMaterial material0,
        PhysicsMaterial material1);
    SurfaceContactInfo* get_contact_info(
        const FixedArray<double, 3>& intersection_point,
        const IntersectionScene& c);
    void advance_time(float dt);
private:
    std::map<UnorderedMaterialPair, SurfaceContactInfo> surface_contact_infos_;
};

}
