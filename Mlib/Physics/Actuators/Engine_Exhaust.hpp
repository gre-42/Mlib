#pragma once
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Actuators/IEngine_Event_Listener.hpp>
#include <Mlib/Physics/Rotating_Frame.hpp>
#include <Mlib/Physics/Smoke_Generation/Constant_Particle_Trail.hpp>
#include <optional>

namespace Mlib {

class SmokeParticleGenerator;

class EngineExhaust: public IEngineEventListener {
public:
    explicit EngineExhaust(
        SmokeParticleGenerator& smoke_generator,
        const ConstantParticleTrail& particle,
        const TransformationMatrix<SceneDir, ScenePos, 3>& relative_location);
    virtual ~EngineExhaust() override;
    virtual void notify_rotation(
        float engine_angular_velocity,
        float tires_angular_velocity,
        const EnginePowerIntent& engine_power_intent,
        float max_surface_power) override;
    virtual void set_location(
        const RotatingFrame<SceneDir, ScenePos, 3>& frame) override;
    virtual void advance_time(float dt) override;
private:
    SmokeParticleGenerator& smoke_generator_;
    ConstantParticleTrail particle_;
    TransformationMatrix<SceneDir, ScenePos, 3> relative_location_;
    std::optional<RotatingFrame<SceneDir, ScenePos, 3>> parent_frame_;
};

}
