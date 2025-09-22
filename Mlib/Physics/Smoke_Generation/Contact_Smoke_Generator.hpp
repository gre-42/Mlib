#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Maybe_Generate.hpp>
#include <Mlib/Physics/Smoke_Generation/Particle_Trail_Generator.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <compare>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>

namespace Mlib {

class SmokeParticleGenerator;
class RigidBodyVehicle;
struct IntersectionScene;
struct SurfaceSmokeInfo;
class OneShotAudio;

struct ContactEmissions {
    MaybeGenerate maybe_generate;
    std::optional<ParticleTrailGenerator> particle_trail_generator;
    std::optional<FixedArray<ScenePos, 3>> old_position;
};

struct ContactSmokeAndAudio {
    std::map<std::pair<size_t, const SurfaceSmokeInfo*>, ContactEmissions> smoke;
    std::map<size_t, ContactEmissions> audio;
};

class ContactSmokeGenerator: public DestructionObserver<const RigidBodyVehicle&>, public virtual DanglingBaseClass {
public:
    ContactSmokeGenerator(
        OneShotAudio& one_shot_audio,
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
    OneShotAudio& one_shot_audio_;
    SmokeParticleGenerator& air_smoke_particle_generator_;
    SmokeParticleGenerator& skidmark_smoke_particle_generator_;
    SmokeParticleGenerator& sea_wave_smoke_particle_generator_;
    std::unordered_map<RigidBodyVehicle*, ContactSmokeAndAudio> tire_smoke_trail_generators_;
};

}
