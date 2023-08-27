#include "Create_Gun.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Stats/Random_Process.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(parent_rigid_body_node);
DECLARE_ARGUMENT(punch_angle_node);
DECLARE_ARGUMENT(cool_down);
DECLARE_ARGUMENT(bullet_renderable);
DECLARE_ARGUMENT(bullet_hitbox);
DECLARE_ARGUMENT(bullet_explosion_resource);
DECLARE_ARGUMENT(bullet_explosion_animation_time);
DECLARE_ARGUMENT(bullet_rigid_body_flags);
DECLARE_ARGUMENT(bullet_mass);
DECLARE_ARGUMENT(bullet_velocity);
DECLARE_ARGUMENT(bullet_lifetime);
DECLARE_ARGUMENT(bullet_damage);
DECLARE_ARGUMENT(bullet_damage_radius);
DECLARE_ARGUMENT(bullet_size);
DECLARE_ARGUMENT(bullet_trail_resource);
DECLARE_ARGUMENT(bullet_trail_dt);
DECLARE_ARGUMENT(bullet_trail_animation_time);
DECLARE_ARGUMENT(ammo_type);
DECLARE_ARGUMENT(punch_angle_idle_std);
DECLARE_ARGUMENT(punch_angle_shoot_std);
DECLARE_ARGUMENT(muzzle_flash_resource);
DECLARE_ARGUMENT(muzzle_flash_position);
DECLARE_ARGUMENT(muzzle_flash_animation_time);
DECLARE_ARGUMENT(generate_muzzle_flash_hider);
DECLARE_ARGUMENT(capture);
}

const std::string CreateGun::key = "gun";

LoadSceneJsonUserFunction CreateGun::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateGun(args.renderable_scene()).execute(args);
};

CreateGun::CreateGun(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
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
    : idle_rng_{ idle_seed, 0.f, idle_std * std::sqrt(2.f / idle_alpha) },
      shoot_rng_{ shoot_seed, 0.f, shoot_std },
      idle_smoother_{ idle_alpha, idle_std },
      decay_{ decay },
      punch_angle_{ idle_std }
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
    NormalRandomNumberGenerator<float> idle_rng_;
    NormalRandomNumberGenerator<float> shoot_rng_;
    ExponentialSmoother<float> idle_smoother_;
    float decay_;
    float punch_angle_;
};

void CreateGun::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    DanglingRef<SceneNode> parent_rb_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::parent_rigid_body_node), DP_LOC);
    auto rb = dynamic_cast<RigidBodyVehicle*>(&parent_rb_node->get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    DanglingRef<SceneNode> punch_angle_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::punch_angle_node), DP_LOC);
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
    auto gun = std::make_unique<Gun>(
        scene,
        scene_node_resources,
        smoke_particle_generator,
        physics_engine.rigid_bodies_,
        physics_engine.advance_times_,
        args.arguments.at<float>(KnownArgs::cool_down) * s,
        *rb,
        node,
        punch_angle_node,
        args.arguments.at<std::string>(KnownArgs::bullet_renderable),
        args.arguments.at<std::string>(KnownArgs::bullet_hitbox),
        args.arguments.at<std::string>(KnownArgs::bullet_explosion_resource),
        args.arguments.at<float>(KnownArgs::bullet_explosion_animation_time) * s,
        rigid_body_vehicle_flags_from_string(args.arguments.at<std::string>(KnownArgs::bullet_rigid_body_flags)),
        args.arguments.at<float>(KnownArgs::bullet_mass) * kg,
        args.arguments.at<float>(KnownArgs::bullet_velocity) * meters / s,
        args.arguments.at<float>(KnownArgs::bullet_lifetime) * s,
        args.arguments.at<float>(KnownArgs::bullet_damage),
        args.arguments.at<float>(KnownArgs::bullet_damage_radius, 0.f) * meters,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::bullet_size) * meters,
        args.arguments.at<std::string>(KnownArgs::bullet_trail_resource, ""),
        args.arguments.at<float>(KnownArgs::bullet_trail_dt, NAN) * s,
        args.arguments.at<float>(KnownArgs::bullet_trail_animation_time, NAN) * s,
        args.arguments.at<std::string>(KnownArgs::ammo_type),
        punch_angle_rng,
        args.arguments.at<std::string>(KnownArgs::muzzle_flash_resource, ""),
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::muzzle_flash_position, fixed_nans<float, 3>()) * meters,
        args.arguments.at<float>(KnownArgs::muzzle_flash_animation_time, NAN) * s,
        [macro_line_executor = args.macro_line_executor,
         macro = args.arguments.try_at(KnownArgs::generate_muzzle_flash_hider),
         capture = args.arguments.try_at(KnownArgs::capture)](const std::string& muzzle_flash_suffix)
        {
            if (!macro.has_value()) {
                return;
            }
            JsonMacroArguments local_substitutions;
            if (capture.has_value()) {
                local_substitutions.insert_json(capture.value());
            }
            local_substitutions.insert_json("MUZZLE_FLASH_SUFFIX", muzzle_flash_suffix);
            macro_line_executor(macro.value(), &local_substitutions, nullptr);
        },
        delete_node_mutex);
        
    linker.link_absolute_observer(node, std::move(gun));
}
