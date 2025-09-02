#pragma once
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Actuators/IEngine_Event_Listener.hpp>
#include <Mlib/Physics/Maybe_Generate.hpp>
#include <Mlib/Physics/Rotating_Frame.hpp>
#include <Mlib/Physics/Smoke_Generation/Constant_Particle_Trail.hpp>
#include <Mlib/Physics/Smoke_Generation/Particle_Trail_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <optional>

namespace Mlib {

class RenderingResources;
class SceneNodeResources;
class IParticleRenderer;
class Scene;

class EngineExhaust: public IEngineEventListener {
public:
    explicit EngineExhaust(
        RenderingResources& rendering_resources,
        SceneNodeResources& scene_node_resources,
        std::shared_ptr<IParticleRenderer> particle_renderer,
        Scene& scene,
        const ConstantParticleTrail& particle,
        const TransformationMatrix<SceneDir, ScenePos, 3>& relative_location,
        float p_reference);
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
    MaybeGenerate maybe_generate_;
    SmokeParticleGenerator smoke_generator_;
    ParticleTrailGenerator trail_generator_;
    ConstantParticleTrail particle_;
    TransformationMatrix<SceneDir, ScenePos, 3> relative_location_;
    float p_reference_;
};

}
