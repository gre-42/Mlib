#include "Create_Gun.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Audio/Audio_Periodicity.hpp>
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/One_Shot_Audio.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Json/Chrono_Time_Point.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Creator.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Storage.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Stats/Random_Process.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <memory>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(parent_rigid_body_node);
DECLARE_ARGUMENT(punch_angle_node);
DECLARE_ARGUMENT(ypln_node);
DECLARE_ARGUMENT(cool_down);
DECLARE_ARGUMENT(bullet_type);
DECLARE_ARGUMENT(ammo_type);
DECLARE_ARGUMENT(generate_smart_bullet);
DECLARE_ARGUMENT(punch_angle_idle_std);
DECLARE_ARGUMENT(punch_angle_shoot_std);
DECLARE_ARGUMENT(generate_muzzle_flash);
DECLARE_ARGUMENT(shot_audio);
}

const std::string CreateGun::key = "gun";

LoadSceneJsonUserFunction CreateGun::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateGun(args.physics_scene()).execute(args);
};

CreateGun::CreateGun(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

class PunchAngleRng {
public:
    PunchAngleRng(
        unsigned int idle_seed,
        unsigned int shoot_seed,
        float idle_std,
        float shoot_std,
        float idle_alpha,
        float decay)
    : idle_rng_{ idle_seed, 0.f, idle_std * std::sqrt(2.f / idle_alpha) }
    , shoot_rng_{ shoot_seed, 0.f, shoot_std }
    , idle_smoother_{ idle_alpha, idle_std }
    , decay_{ decay }
    , punch_angle_{ idle_std }
    {}
    float operator () (bool shooting) {
        punch_angle_ += idle_smoother_(idle_rng_());
        if (shooting) {
            punch_angle_ += shoot_rng_();
        }
        punch_angle_ *= (1 - decay_);
        return punch_angle_;
    }
private:
    FastNormalRandomNumberGenerator<float> idle_rng_;
    FastNormalRandomNumberGenerator<float> shoot_rng_;
    ExponentialSmoother<float> idle_smoother_;
    float decay_;
    float punch_angle_;
};

void CreateGun::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    DanglingBaseClassRef<SceneNode> parent_rb_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::parent_rigid_body_node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(parent_rb_node);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    DanglingBaseClassPtr<SceneNode> punch_angle_node = args.arguments.contains_non_null(KnownArgs::punch_angle_node)
        ? scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::punch_angle_node), DP_LOC).ptr()
        : nullptr;
    float punch_angle_idle_std = args.arguments.at<float>(KnownArgs::punch_angle_idle_std) * degrees;
    float punch_angle_shoot_std = args.arguments.at<float>(KnownArgs::punch_angle_shoot_std) * degrees;
    float punch_angle_idle_alpha = 0.002f;
    float decay = 0.05f;
    // octave> a=0.002; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9980
    // octave> a=0.004; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9960
    // => var = a / 2, std = sqrt(a / 2)
    PunchAngleRng pitch_rng(0, 1, punch_angle_idle_std, punch_angle_shoot_std, punch_angle_idle_alpha, decay);
    PunchAngleRng yaw_rng(2, 3, punch_angle_idle_std, punch_angle_shoot_std, punch_angle_idle_alpha, decay);
    std::function<FixedArray<float, 3>(bool shooting)> punch_angle_rng{
        [pitch_rng, yaw_rng](bool shooting) mutable {
            return FixedArray<float, 3>{pitch_rng(shooting), yaw_rng(shooting), 0.f};
        }};
    const auto& bullet_props = args.bullet_property_db.get(args.arguments.at<std::string>(KnownArgs::bullet_type));
    ITrailStorage* bullet_trace_storage = nullptr;
    if (!bullet_props.trace_storage->empty()) {
        bullet_trace_storage = &trail_renderer.get_storage(bullet_props.trace_storage);
    }
    std::function<void(
        const std::optional<std::string>& player,
        const std::string& bullet_suffix,
        const std::optional<VariableAndHash<std::string>>& target,
        const FixedArray<float, 3>& velocity,
        const FixedArray<float, 3>& angular_velocity)> generate_smart_bullet;
    if (auto g = args.arguments.try_at(KnownArgs::generate_smart_bullet); g.has_value()) {
        generate_smart_bullet =
            [mle = args.macro_line_executor,
             l = *g]
            (
                const std::optional<std::string>& player,
                const std::string& bullet_suffix,
                const std::optional<VariableAndHash<std::string>>& target,
                const FixedArray<float, 3>& velocity,
                const FixedArray<float, 3>& angular_velocity)
            {
                nlohmann::json let{
                    {"bullet_player_name", player.has_value() ? nlohmann::json(*player) : nlohmann::json()},
                    {"bullet_target", target.has_value() ? nlohmann::json(*target) : nlohmann::json()},
                    {"bullet_suffix", bullet_suffix},
                    {"bullet_velocity", velocity / kph},
                    {"bullet_angular_velocity", angular_velocity / rpm},
                };
                mle.inserted_block_arguments(let)(l, nullptr);
            };
    }
    std::function<void(const AudioSourceState<ScenePos>&)> generate_shot_audio;
    if (auto a = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::shot_audio); a.has_value()) {
        generate_shot_audio =
        [
            &o=one_shot_audio,
            audio_buffer=AudioResourceContextStack::primary_audio_resources()->get_buffer(*a),
            &audio_meta=AudioResourceContextStack::primary_audio_resources()->get_buffer_meta(*a)
        ]
        (const AudioSourceState<ScenePos>& state)
        {
            o.play(*audio_buffer, audio_meta.lowpass.get(), state, AudioPeriodicity::APERIODIC, audio_meta.distance_clamping, audio_meta.gain);
        };
    }
    std::function<void(const AudioSourceState<ScenePos>&)> generate_bullet_explosion_audio;
    if (const auto& a = bullet_props.explosion_audio_resource_name; !a->empty()) {
        generate_bullet_explosion_audio =
        [
            &o=one_shot_audio,
            audio_buffer=AudioResourceContextStack::primary_audio_resources()->get_buffer(a),
            &audio_meta=AudioResourceContextStack::primary_audio_resources()->get_buffer_meta(a)
        ]
        (const AudioSourceState<ScenePos>& state)
        {
            o.play(*audio_buffer, audio_meta.lowpass.get(), state, AudioPeriodicity::APERIODIC, audio_meta.distance_clamping, audio_meta.gain);
        };
    }
    std::function<UpdateAudioSourceState(const AudioSourceState<ScenePos>&)> generate_bullet_engine_audio;
    if (const auto& a = bullet_props.engine_audio_resource_name; !a->empty()) {
        generate_bullet_engine_audio =
        [
            &o=one_shot_audio,
            audio_buffer=AudioResourceContextStack::primary_audio_resources()->get_buffer(a),
            &audio_meta=AudioResourceContextStack::primary_audio_resources()->get_buffer_meta(a)
        ]
        (const AudioSourceState<ScenePos>& state) -> UpdateAudioSourceState
        {
            auto asp = o.play(*audio_buffer, audio_meta.lowpass.get(), state, AudioPeriodicity::PERIODIC, audio_meta.distance_clamping, audio_meta.gain);
            return [asp](const AudioSourceState<ScenePos>* state){
                if (state == nullptr) {
                    asp->source.stop();
                } else {
                    asp->position = *state;
                }
            };
        };
    }
    std::function<void(const StaticWorld&)> generate_muzzle_flash;
    if (auto macro = args.arguments.try_at_non_null(KnownArgs::generate_muzzle_flash); macro.has_value()) {
        generate_muzzle_flash =
        [
            macro=*macro,
            mle=args.macro_line_executor
        ](const StaticWorld& world)
        {
            nlohmann::json let{
                {"time_point", world.time}
            };
            mle.inserted_block_arguments(let)(macro, nullptr);
        };
    }
    auto& gun = global_object_pool.create<Gun>(
        CURRENT_SOURCE_LOCATION,
        &rendering_resources,
        scene,
        scene_node_resources,
        air_particles.smoke_particle_generator,
        dynamic_lights,
        physics_engine.rigid_bodies_,
        physics_engine.advance_times_,
        args.arguments.at<float>(KnownArgs::cool_down) * seconds,
        rb,
        node,
        punch_angle_node,
        bullet_props,
        std::move(generate_smart_bullet),
        std::move(generate_shot_audio),
        std::move(generate_bullet_explosion_audio),
        std::move(generate_bullet_engine_audio),
        bullet_trace_storage,
        args.arguments.at<std::string>(KnownArgs::ammo_type),
        punch_angle_rng,
        generate_muzzle_flash);
    if (args.arguments.contains_non_null(KnownArgs::ypln_node)) {
        auto ypln_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::ypln_node), DP_LOC);
        gun.set_ypln_node(ypln_node);
    }
}
