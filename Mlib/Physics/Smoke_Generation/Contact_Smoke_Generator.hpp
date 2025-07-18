#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Smoke_Generation/Particle_Trail_Generator.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <compare>
#include <map>
#include <string>
#include <unordered_map>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SmokeParticleGenerator;
class RigidBodyVehicle;
struct IntersectionScene;
struct SurfaceSmokeInfo;

class ContactSmokeGenerator: public DestructionObserver<const RigidBodyVehicle&>, public virtual DanglingBaseClass {
public:
    ContactSmokeGenerator(
        SmokeParticleGenerator& air_smoke_particle_generator,
        SmokeParticleGenerator& skidmark_smoke_particle_generator,
        SmokeParticleGenerator& sea_wave_smoke_particle_generator);
    ~ContactSmokeGenerator();
    virtual void notify_destroyed(const RigidBodyVehicle& destroyed_object) override;

    void notify_contact(
        const FixedArray<ScenePos, 3>& intersection_point,
        const FixedArray<float, 3>& rotation,
        const FixedArray<SceneDir, 3>& surface_normal,
        const IntersectionScene& c);
    void advance_time(float dt);
private:
    SmokeParticleGenerator& air_smoke_particle_generator_;
    SmokeParticleGenerator& skidmark_smoke_particle_generator_;
    SmokeParticleGenerator& sea_wave_smoke_particle_generator_;
    std::unordered_map<RigidBodyVehicle*, std::map<std::pair<size_t, const SurfaceSmokeInfo*>, ParticleTrailGenerator>> tire_smoke_trail_generators_;
};

}
