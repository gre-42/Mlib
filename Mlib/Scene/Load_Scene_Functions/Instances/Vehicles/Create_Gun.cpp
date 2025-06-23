#include "Create_Gun.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Storage.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Stats/Random_Process.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(parent_rigid_body_node);
DECLARE_ARGUMENT(punch_angle_node);
DECLARE_ARGUMENT(cool_down);
DECLARE_ARGUMENT(bullet_type);
DECLARE_ARGUMENT(ammo_type);
DECLARE_ARGUMENT(generate_smart_bullet);
DECLARE_ARGUMENT(punch_angle_idle_std);
DECLARE_ARGUMENT(punch_angle_shoot_std);
DECLARE_ARGUMENT(muzzle_flash_resource);
DECLARE_ARGUMENT(muzzle_flash_position);
DECLARE_ARGUMENT(muzzle_flash_animation_time);
DECLARE_ARGUMENT(generate_muzzle_flash_hider);
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
    DanglingRef<SceneNode> parent_rb_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::parent_rigid_body_node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(parent_rb_node);
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    DanglingPtr<SceneNode> punch_angle_node = args.arguments.contains_non_null(KnownArgs::punch_angle_node)
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
    global_object_pool.create<Gun>(
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
        bullet_trace_storage,
        args.arguments.at<std::string>(KnownArgs::ammo_type),
        punch_angle_rng,
        VariableAndHash{ args.arguments.at<std::string>(KnownArgs::muzzle_flash_resource, "") },
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::muzzle_flash_position, fixed_nans<float, 3>()) * meters,
        args.arguments.at<float>(KnownArgs::muzzle_flash_animation_time, NAN) * seconds,
        [macro_line_executor = args.macro_line_executor,
         macro = args.arguments.try_at_non_null(KnownArgs::generate_muzzle_flash_hider)](const std::string& muzzle_flash_suffix)
        {
            if (!macro.has_value()) {
                return;
            }
            auto mle = macro_line_executor.inserted_block_arguments({{"muzzle_flash_suffix", muzzle_flash_suffix}});
            mle(*macro, nullptr);
        });
}
