#include "Create_Damageable.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Audio/Audio_Periodicity.hpp>
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/One_Shot_Audio.hpp>
#include <Mlib/Macro_Executor/Asset_Group_And_Id.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Translator.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Deleting_Damageable.hpp>
#include <Mlib/Physics/Interfaces/Damage_Source.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownExplosionArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(damage_sources);
DECLARE_ARGUMENT(animation);
DECLARE_ARGUMENT(animation_duration);
DECLARE_ARGUMENT(particle_container);
DECLARE_ARGUMENT(audio);
};

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(health);
DECLARE_ARGUMENT(explosions);
DECLARE_ARGUMENT(delete_node_when_health_leq_zero);
}

CreateDamageable::CreateDamageable(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateDamageable::execute(const JsonView& args)
{
    args.validate(KnownArgs::options);

    auto node = args.at<VariableAndHash<std::string>>(KnownArgs::node);

    DeletingDamageable::GenerateExplosions generate_explosions;
    auto explosions = args.try_at<std::vector<nlohmann::json>>(KnownArgs::explosions);

    if (explosions.has_value()) {
        generate_explosions.reserve(explosions->size());
        for (const auto& e : *explosions) {
            JsonView ev{ e };
            ev.validate(KnownExplosionArgs::options);
            std::function<void(const AudioSourceState<ScenePos>&, const StaticWorld&)> generate_explosion_animation;
            if (auto resource = ev.try_at<VariableAndHash<std::string>>(KnownExplosionArgs::animation); resource.has_value()) {
                generate_explosion_animation = [
                    &g = air_particles.smoke_particle_generator,
                    resource = *resource,
                    node = *node,
                    &scene = scene,
                    duration = ev.at<float>(KnownExplosionArgs::animation_duration) * seconds,
                    particle_container = ev.at<ParticleContainer>(KnownExplosionArgs::particle_container)
                ]
                (const AudioSourceState<ScenePos>& state, const StaticWorld& static_world)
                {
                    g.generate_root(
                        resource,
                        VariableAndHash<std::string>{node + scene.get_temporary_instance_suffix()},
                        state.position,
                        fixed_zeros<float, 3>(),
                        fixed_zeros<float, 3>(),
                        0.f,
                        0.f,
                        duration,
                        particle_container,
                        static_world);
                };
            }
            std::function<void(const AudioSourceState<ScenePos>&, const StaticWorld&)> generate_explosion_audio;
            if (auto a = ev.try_at<VariableAndHash<std::string>>(KnownExplosionArgs::audio); a.has_value()) {
                generate_explosion_audio =
                [
                    &o=one_shot_audio,
                    shot_audio_buffer=AudioResourceContextStack::primary_audio_resources()->get_buffer(*a),
                    &shot_audio_meta=AudioResourceContextStack::primary_audio_resources()->get_buffer_meta(*a)
                ]
                (const AudioSourceState<ScenePos>& state, const StaticWorld& static_world)
                {
                    o.play(*shot_audio_buffer, shot_audio_meta.lowpass.get(), state, AudioPeriodicity::APERIODIC, shot_audio_meta.distance_clamping, shot_audio_meta.gain);
                };
            }
            std::function<void(const AudioSourceState<ScenePos>&, const StaticWorld&)> generate_explosion =
                [animation=std::move(generate_explosion_animation),
                audio=std::move(generate_explosion_audio)]
                (const AudioSourceState<ScenePos>& state, const StaticWorld& static_world)
            {
                if (animation) {
                    animation(state, static_world);
                }
                if (audio) {
                    audio(state, static_world);
                }
            };
            auto damage_sources = ev.at<DamageSource>(KnownExplosionArgs::damage_sources, DamageSource::ANY);
            generate_explosions.emplace_back(damage_sources, std::move(generate_explosion));
        }
    }
    global_object_pool.create<DeletingDamageable>(
        CURRENT_SOURCE_LOCATION,
        scene,
        physics_engine.advance_times_,
        node,
        args.at<float>(KnownArgs::health),
        args.at<bool>(KnownArgs::delete_node_when_health_leq_zero),
        translator,
        std::move(generate_explosions));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "damageable",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateDamageable(args.physics_scene()).execute(args.arguments);
            });
    }
} obj;

}
