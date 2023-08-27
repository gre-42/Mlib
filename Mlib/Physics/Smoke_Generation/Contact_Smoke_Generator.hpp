#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Trail_Generator.hpp>
#include <compare>
#include <string>
#include <unordered_map>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SmokeParticleGenerator;
class RigidBodyVehicle;
struct IntersectionScene;
struct SurfaceContactInfo;
class SurfaceContactDb;

class ContactSmokeGenerator: public DestructionObserver<const RigidBodyVehicle&> {
public:
    explicit ContactSmokeGenerator(
        SurfaceContactDb& surface_contact_db,
        SmokeParticleGenerator& smoke_particle_generator);
    ~ContactSmokeGenerator();
    virtual void notify_destroyed(const RigidBodyVehicle& destroyed_object) override;

    SurfaceContactInfo* notify_contact(
        const FixedArray<double, 3>& intersection_point,
        const IntersectionScene& c);
    void advance_time(float dt);
private:
    SurfaceContactDb& surface_contact_db_;
    SmokeParticleGenerator& smoke_particle_generator_;
    std::unordered_map<RigidBodyVehicle*, std::unordered_map<size_t, SmokeTrailGenerator>> tire_smoke_trail_generators_;
};

}
