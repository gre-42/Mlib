#include "Create_Generic_Avatar.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Actuators/Engine_Event_Listeners.hpp>
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Advance_Times/Crash.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Wheel.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Pacejkas_Magic_Formula.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Misc/When_To_Equip.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Type.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Avatar_As_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Avatar_As_Car_Controller.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Scene/Animation/Avatar_Animation_Updater.hpp>
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Child_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Insert_Physics_Node_Hider.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Child_Renderable_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Add_Weapon_To_Cycle.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Damageable.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Gun.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rigid_Cuboid.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rigid_Disk.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Stats/Random_Process.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

using VH = VariableAndHash<std::string>;
static const auto WORLD = VariableAndHash<std::string>{"world"};

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(suffix);
DECLARE_ARGUMENT(if_with_graphics);
DECLARE_ARGUMENT(if_with_physics);
DECLARE_ARGUMENT(if_human_style);
DECLARE_ARGUMENT(if_damageable);
DECLARE_ARGUMENT(color);
DECLARE_ARGUMENT(parking_brake_pulled);
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(angular_velocity);
DECLARE_ARGUMENT(mute);
DECLARE_ARGUMENT(with_gun);
DECLARE_ARGUMENT(velocity_error_std);
DECLARE_ARGUMENT(error_alpha);
DECLARE_ARGUMENT(locked_on_angle);
DECLARE_ARGUMENT(yaw_error_std);
DECLARE_ARGUMENT(pitch_error_std);
DECLARE_ARGUMENT(pitch_min);
DECLARE_ARGUMENT(pitch_max);
DECLARE_ARGUMENT(dpitch_max);
DECLARE_ARGUMENT(dyaw_max);
DECLARE_ARGUMENT(steering_multiplier);
DECLARE_ARGUMENT(animation_resource_wo_gun);
DECLARE_ARGUMENT(animation_resource_w_gun);
DECLARE_ARGUMENT(y_fov);
DECLARE_ARGUMENT(near_plane);
DECLARE_ARGUMENT(far_plane);
}

namespace KnownDb {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(decimate);
DECLARE_ARGUMENT(decimate1);
DECLARE_ARGUMENT(wheel_mount_0);
DECLARE_ARGUMENT(wheel_mount_1);
DECLARE_ARGUMENT(angular_vels);
DECLARE_ARGUMENT(angular_vels_rear);
DECLARE_ARGUMENT(angular_vels_front);
DECLARE_ARGUMENT(powers);
DECLARE_ARGUMENT(powers_rear);
DECLARE_ARGUMENT(powers_front);
DECLARE_ARGUMENT(gear_ratios);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(com);
DECLARE_ARGUMENT(mass);
DECLARE_ARGUMENT(wheel);
DECLARE_ARGUMENT(w_clutch);
DECLARE_ARGUMENT(max_dw);
DECLARE_ARGUMENT(engine_audio);
DECLARE_ARGUMENT(waypoint_dy);
}

namespace KnownAudio {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(p_idle);
DECLARE_ARGUMENT(p_reference);
}

namespace KnownExhaust {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(location);
DECLARE_ARGUMENT(particle);
DECLARE_ARGUMENT(p_reference);
}

namespace KnownWheels {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(musF);
DECLARE_ARGUMENT(musC);
DECLARE_ARGUMENT(brake_force);
DECLARE_ARGUMENT(brake_torque);
DECLARE_ARGUMENT(Ks);
DECLARE_ARGUMENT(Ka);
DECLARE_ARGUMENT(Ke);
}

static inline float stow(float v) {
    return v * rpm;
}

static inline float stop(float v) {
    return v * hp;
}

CreateGenericAvatar::CreateGenericAvatar(
    PhysicsScene& physics_scene,
    const MacroLineExecutor& macro_line_executor)
    : LoadPhysicsSceneInstanceFunction{ physics_scene, &macro_line_executor }
{}

void CreateGenericAvatar::execute(const JsonView& args)
{
    args.validate(KnownArgs::options);

    auto create_child_node = CreateChildNode{ physics_scene };
    auto child_renderable_instance = ChildRenderableInstance{ physics_scene };
    auto create_rigid_cuboid = CreateRigidCuboid{ physics_scene };
    auto create_rigid_disk = CreateRigidDisk{ physics_scene };
    auto add_weapon_to_cycle = AddWeaponToInventory{ physics_scene, macro_line_executor };
    auto create_gun = CreateGun{ physics_scene, macro_line_executor };
    auto create_damageable = CreateDamageable{ physics_scene };

    auto asset_id = args.at<std::string>(KnownArgs::asset_id);
    auto suffix = args.at<std::string>(KnownArgs::suffix);
    auto if_with_graphics = args.at<bool>(KnownArgs::if_with_graphics);
    auto if_with_physics = args.at<bool>(KnownArgs::if_with_physics);
    auto if_human_style = args.at<bool>(KnownArgs::if_human_style);
    auto with_gun = args.at<bool>(KnownArgs::with_gun);
    const auto& vdb = asset_references["vehicles"].at(asset_id).rp.database;
    auto wheel = vdb.at<std::string>(KnownDb::wheel);
    const auto& wdb = asset_references["wheels"].at(wheel).rp.database;
    auto decimate = vdb.at<std::string>(KnownDb::decimate);
    auto decimate1 = vdb.at<std::string>(KnownDb::decimate1);
    auto parent = VH{"human_node" + suffix};
    auto animation_node = VH{"animation_node" + suffix};
    auto main_gun_node = VH{"main_gun_node" + suffix};
    auto main_gun_punch_angle_node = VH{"main_gun_punch_angle_node" + suffix};
    auto main_gun_end_node = VH{"main_gun_end_node" + suffix};
    auto main_gun_node_visual0 = VH{"main_gun_node_visual0" + suffix};
    auto main_gun_node_visual = VH{"main_gun_node_visual" + suffix};
    auto human_head_node = VH{"human_head_node" + suffix};
    auto human_head_node2 = VH{"human_head_node2" + suffix};
    auto human_head_camera_node = VH{"human_head_camera_node" + suffix};

    auto wheel_mount_0 = vdb.at<EFixedArray<float, 3>>(KnownDb::wheel_mount_0);
    auto wheel_mount_1 = vdb.at<EFixedArray<float, 3>>(KnownDb::wheel_mount_1);

    DanglingBaseClassRef<SceneNode> parent_node = scene.get_node(parent, DP_LOC);

    if (if_human_style) {
        auto style = std::unique_ptr<ColorStyle>(new ColorStyle{
            .ambient = args.at<EOrderableFixedArray<float, 3>>(KnownArgs::color),
            .diffuse = args.at<EOrderableFixedArray<float, 3>>(KnownArgs::color)});
        parent_node->add_color_style(std::move(style));

        auto animation_loop_name = with_gun
            ? VH{args.at<std::string>(KnownArgs::animation_resource_w_gun) + ".idle"}
            : VH{args.at<std::string>(KnownArgs::animation_resource_wo_gun) + ".idle"};

        float animation_loop_end = RenderingContextStack::primary_scene_node_resources()
            .get_animation_duration(animation_loop_name);
        auto animation_state = std::unique_ptr<AnimationState>(new AnimationState{
            .periodic_skelletal_animation_name = animation_loop_name,
            .periodic_skelletal_animation_frame = {
                AnimationFrame{
                    .begin = 0 * seconds,
                    .end = animation_loop_end * seconds,
                    .time = 0 * seconds}}});
        parent_node->set_animation_state(
            std::move(animation_state),
            AnimationStateAlreadyExistsBehavior::THROW);
    }
    
    create_child_node("dynamic", parent, animation_node, {0.f, -0.5f * meters, 0.f}, {0.f, 180.f * degrees, 0.f});
    // The human hitbox has width and length of 0.5 (or \"radius\" or 0.25), so z = -0.3 is out of its hitbox"
    create_child_node("dynamic", parent, main_gun_node, {0.1f * meters, 0.5f * meters, 0.f});
    // -0.5 is inside the opponent's hitbox ending at -0.25 - 0.5"
    create_child_node("dynamic", main_gun_node            , main_gun_punch_angle_node, {0.f, 0.f, -0.5f * meters});
    create_child_node("dynamic", main_gun_punch_angle_node, main_gun_end_node        , {0.f, 0.f, 0.f});
    create_child_node("dynamic", animation_node           , human_head_node          , {0.f, 1.7f * meters, 0.f});
    create_child_node("dynamic", human_head_node          , human_head_node2         , {0.f, 0.f, 0.f}, {0.f, 180 * degrees, 0.f});
    create_child_node("dynamic", human_head_node2         , human_head_camera_node   , {0.f, 0.f, 0.f});
    
    {
        DanglingBaseClassRef<SceneNode> camera_node = scene.get_node(human_head_camera_node, DP_LOC);
        {
            auto pc = std::make_unique<PerspectiveCamera>(
                PerspectiveCameraConfig(),
                PerspectiveCamera::Postprocessing::ENABLED);
            pc->set_y_fov(args.at<float>(KnownArgs::y_fov) * degrees);
            pc->set_near_plane(args.at<float>(KnownArgs::near_plane));
            pc->set_far_plane(args.at<float>(KnownArgs::far_plane));
            pc->set_requires_postprocessing(true);
            camera_node->set_camera(std::move(pc));
        }
        {
            auto node_hider = std::make_unique<PhysicsNodeHiderWithEvent>(
                parent_node,
                camera_node);
            auto& nh = global_object_pool.add(std::move(node_hider), CURRENT_SOURCE_LOCATION);
            parent_node->clearing_observers.add({ nh, CURRENT_SOURCE_LOCATION });
            camera_node->clearing_observers.add({ nh, CURRENT_SOURCE_LOCATION });
            parent_node->insert_node_hider(nullptr, { nh, CURRENT_SOURCE_LOCATION });
            physics_engine.advance_times_.add_advance_time({ nh, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
        }
    }

    if (if_with_graphics) {
        create_child_node("dynamic", animation_node, main_gun_node_visual0, {0.f, 0.f, 0.f});
        create_child_node("dynamic", main_gun_node_visual0, main_gun_node_visual, {-0.075f * meters, 0.f, 0.025f * meters}, {90 * degrees, 0.f, 90 * degrees});
    }
    if (if_with_physics) {
        auto& rb = create_rigid_cuboid(CreateRigidCuboidArgs{
            .node = parent,
            .name = "generic_avatar_" + asset_id + suffix,
            .asset_id = asset_id,
            .mass = vdb.at<float>(KnownDb::mass) * kg,
            .size = fixed_full<float, 3>(INFINITY * meters),
            .com = vdb.at<EFixedArray<float, 3>>(KnownDb::com) * meters,
            .v = args.at<EFixedArray<float, 3>>(KnownArgs::velocity) * kph,
            .w = args.at<EFixedArray<float, 3>>(KnownArgs::angular_velocity) * rpm,
            .I_rotation = fixed_zeros<float, 3>(),
            .with_penetration_limits = true,
            .geographic_coordinates = scene_node_resources.get_geographic_mapping(WORLD),
            .flags = RigidBodyVehicleFlags::IS_ACTIVATED_AVATAR,
            .waypoint_dy = vdb.at<CompressedScenePos>(KnownDb::waypoint_dy),
            .hitboxes = VariableAndHash<std::string>{"human_hitboxes"},
            .collidable_mode = CollidableMode::COLLIDE | CollidableMode::MOVE});
        
        DanglingBaseClassRef<SceneNode> gun_node = scene.get_node(main_gun_end_node, DP_LOC);
        auto& aim_at = [&]() -> AimAt& {
            float velocity_error_std = args.at<float>(KnownArgs::velocity_error_std);
            float error_alpha = (velocity_error_std != 0.f)
                ? args.at<float>(KnownArgs::error_alpha)
                : 1.f;
            // octave> a=0.002; a/sum((a * (1 - a).^(0 : 100000)).^2)
            // ans = 1.9980
            // octave> a=0.004; a/sum((a * (1 - a).^(0 : 100000)).^2)
            // ans = 1.9960
            // => var = a / 2, std = sqrt(a / 2)
            auto velocity_estimation_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
                FastNormalRandomNumberGenerator<float>{ 0, 0.f, velocity_error_std * std::sqrt(2.f / error_alpha) },
                ExponentialSmoother<float>{ error_alpha, velocity_error_std } };

            return global_object_pool.create<AimAt>(
                CURRENT_SOURCE_LOCATION,
                physics_engine.advance_times_,
                parent_node,
                gun_node,
                0.f,                                    // bullet_start_offset,
                NAN,                                    // bullet_velocity
                true,                                   // bullet_feels_gravity
                9.8f * meters / squared(seconds),       // gravity
                std::cos(args.at<float>(KnownArgs::locked_on_angle) * degrees),
                velocity_estimation_error);
        }();
        {
            Linker linker{ physics_engine.advance_times_ };
            DanglingBaseClassRef<SceneNode> yaw_node = parent_node;
            DanglingBaseClassRef<SceneNode> pitch_node = scene.get_node(main_gun_node, DP_LOC);
            float yaw_error_std = args.at<float>(KnownArgs::yaw_error_std);
            float pitch_velocity_error_std = args.at<float>(KnownArgs::pitch_error_std);
            float error_alpha = (pitch_velocity_error_std != 0.f)
                ? args.at<float>(KnownArgs::error_alpha)
                : 1.f;
            // octave> a=0.002; a/sum((a * (1 - a).^(0 : 100000)).^2)
            // ans = 1.9980
            // octave> a=0.004; a/sum((a * (1 - a).^(0 : 100000)).^2)
            // ans = 1.9960
            // => var = a / 2, std = sqrt(a / 2)
            auto increment_yaw_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
                FastNormalRandomNumberGenerator<float>{ 0, 0.f, yaw_error_std * std::sqrt(2.f / error_alpha) },
                ExponentialSmoother<float>{ error_alpha, yaw_error_std } };
            auto increment_pitch_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
                FastNormalRandomNumberGenerator<float>{ 0, 0.f, pitch_velocity_error_std * std::sqrt(2.f / error_alpha) },
                ExponentialSmoother<float>{ error_alpha, pitch_velocity_error_std } };

            auto ufollower_pitch = std::make_unique<PitchLookAtNode>(
                aim_at,
                args.at<float>(KnownArgs::pitch_min) * degrees,
                args.at<float>(KnownArgs::pitch_max) * degrees,
                args.at<float>(KnownArgs::dpitch_max) * degrees,
                increment_pitch_error);
            auto ufollower = std::make_unique<YawPitchLookAtNodes>(
                aim_at,
                *ufollower_pitch,
                args.at<float>(KnownArgs::dyaw_max) * degrees,
                increment_yaw_error);
            ufollower->pitch_look_at_node().set_head_node(scene.get_node(human_head_camera_node, DP_LOC));
            linker.link_relative_movable(
                yaw_node,
                DanglingBaseClassRef<YawPitchLookAtNodes>{ *ufollower, CURRENT_SOURCE_LOCATION },
                CURRENT_SOURCE_LOCATION);
            linker.link_relative_movable(
                pitch_node,
                DanglingBaseClassRef<PitchLookAtNode>{ *ufollower_pitch, CURRENT_SOURCE_LOCATION },
                CURRENT_SOURCE_LOCATION);
            auto& follower = *ufollower;
            global_object_pool.add(std::move(ufollower), CURRENT_SOURCE_LOCATION);
            global_object_pool.add(std::move(ufollower_pitch), CURRENT_SOURCE_LOCATION);
            rb.vehicle_controller_ = std::make_unique<AvatarAsCarController>(
                rb,
                follower,
                args.at<float>(KnownArgs::steering_multiplier));
            rb.avatar_controller_ = std::make_unique<AvatarAsAvatarController>(rb, follower);
        }

        if (with_gun) {
            auto uweapon_cycle = std::make_unique<WeaponCycle>();
            auto& weapon_cycle = *uweapon_cycle;
            parent_node->set_node_modifier(std::move(uweapon_cycle));
            rb.inventory_.set_capacity(VH{"m4a1_ammo"}, 200);
            rb.inventory_.set_capacity(VH{"beretta92_ammo"}, 200);
            rb.inventory_.set_capacity(VH{"frag_grenade"}, 10);
            rb.inventory_.set_capacity(VH{"m72_law_rocket"}, 2);
            rb.inventory_.set_capacity(VH{"none_weapon_ammo_type"}, 0);

            add_weapon_to_cycle(JsonView{{
                {"cycle_node", *parent},
                {"entry_name", "m4a1"},
                {"ammo_type", "m4a1_ammo"},
                {"bullet_type", "m4a1_bullet"},
                {"cool_down", 0.1},
                {"range_min", 0},
                {"range_max", 2000},
                {"create_weapon", nlohmann::json::object({
                    {"playback", "add_weapon_m4a1"}
                })}
            }});
            add_weapon_to_cycle(JsonView{{
                {"cycle_node", *parent},
                {"entry_name", "beretta92"},
                {"ammo_type", "beretta92_ammo"},
                {"bullet_type", "beretta92_bullet"},
                {"cool_down", 0.5},
                {"range_min", 0},
                {"range_max", 2000},
                {"create_weapon", nlohmann::json::object({
                    {"playback", "add_weapon_beretta92"}
                })}
            }});
            add_weapon_to_cycle(JsonView{{
                {"cycle_node", *parent},
                {"entry_name", "frag_grenade"},
                {"ammo_type", "frag_grenade"},
                {"bullet_type", "frag_grenade"},
                {"cool_down", 0.5},
                {"range_min", 8},
                {"range_max", 20},
                {"create_weapon", nlohmann::json::object({
                    {"playback", "add_weapon_frag_grenade"}
                })}
            }});
            add_weapon_to_cycle(JsonView{{
                {"cycle_node", *parent},
                {"entry_name", "m72_law"},
                {"ammo_type", "m72_law_rocket"},
                {"bullet_type", "m72_law_rocket"},
                {"cool_down", 0.5},
                {"range_min", 8},
                {"range_max", 2000},
                {"create_weapon", nlohmann::json::object({
                    {"playback", "add_weapon_m72_law"}
                })}
            }});

            weapon_cycle.set_desired_weapon(std::nullopt, "m4a1", WhenToEquip::EQUIP_INSTANTLY);
        } else {
            create_gun(JsonView{{
                {"node", *main_gun_end_node},
                {"punch_angle_node", *main_gun_punch_angle_node},
                {"ypln_node", *parent},
                {"cool_down", INFINITY},
                {"punch_angle_idle_std", 0},
                {"punch_angle_shoot_std", 0},
                {"parent_rigid_body_node", *parent},
                {"ammo_type", "none_weapon_ammo_type"},
                {"bullet_type", "none_weapon_bullet_type"}
            }});
        }

        std::shared_ptr<EngineEventListeners> engine_listeners;
        auto add_engine_listener = [&](std::shared_ptr<IEngineEventListener> l){
            if (engine_listeners == nullptr) {
                engine_listeners = std::make_shared<EngineEventListeners>();
            }
            engine_listeners->add(std::move(l));
        };

        if (!args.at<bool>(KnownArgs::mute)) {
            if (auto engine_audio = vdb.try_at_non_null<std::string>(KnownDb::engine_audio); engine_audio.has_value()) {
                const auto& adb = asset_references["engine_audio"].at(*engine_audio).rp.database;
                adb.validate(KnownAudio::options);
                add_engine_listener(std::make_shared<EngineAudio>(
                    adb.at<std::string>(KnownAudio::name),
                    paused,
                    paused_changed,
                    adb.at<float>(KnownAudio::p_idle) * hp,
                    adb.at<float>(KnownAudio::p_reference) * hp));
            }
        }

        auto engine = VariableAndHash<std::string>("legs");
        if (vdb.contains_non_null(KnownDb::powers)) {
            auto engine_power = EnginePower{
                Interp<float>{
                    vdb.at_vector<float>(KnownDb::angular_vels, stow),
                    vdb.at_vector<float>(KnownDb::powers, stop),
                    OutOfRangeBehavior::CLAMP},
                vdb.at<std::vector<float>>(KnownDb::gear_ratios),
                vdb.at<float>(KnownDb::w_clutch) * rpm,
                vdb.at<float>(KnownDb::max_dw, INFINITY) * rpm / seconds };
            rb.engines_.add(
                engine,
                std::move(engine_power),
                engine_listeners);
        } else {
            rb.engines_.add(
                engine,
                std::nullopt,   // power
                nullptr);       // listeners
        }
        if (args.at<bool>(KnownArgs::parking_brake_pulled)) {
            rb.park_vehicle();
        }

        wdb.validate(KnownWheels::options);
        auto wheel_radius = wdb.at<float>(KnownWheels::radius) * meters;
        auto wheel_brake_force = wdb.at<float>(KnownWheels::brake_force) * N;
        auto wheel_brake_torque = wdb.at<float>(KnownWheels::brake_torque) * N * meters;
        auto wheel_Ks = wdb.at<float>(KnownWheels::Ks) * N / meters;
        auto wheel_Ka = wdb.at<float>(KnownWheels::Ka) * N / (meters / seconds);
        auto wheel_Ke = wdb.at<float>(KnownWheels::Ke);
        Interp<float> mus{
            wdb.at<std::vector<float>>(KnownWheels::musF),
            wdb.at<std::vector<float>>(KnownWheels::musC),
            OutOfRangeBehavior::CLAMP};
        auto create_wheel = [&](
            size_t tire_id,
            const VariableAndHash<std::string>& engine,
            const std::optional<VariableAndHash<std::string>>& delta_engine,
            const FixedArray<float, 3>& vehicle_mount_0,
            const FixedArray<float, 3>& vehicle_mount_1)
            {
                // From: https://www.nanolounge.de/21977/federkonstante-und-masse-bei-auto
                // Ds = 1000 / 4 * 9.8 / 0.02 = 122500 = 1.225e5
                // Da * 1 = 1000 / 4 * 9.8 => Da = 1e4 / 4
                rb.tires_.add(
                    tire_id,
                    engine,
                    delta_engine,
                    nullptr,            // wheel_rbp
                    wheel_brake_force,
                    wheel_brake_torque,
                    wheel_Ks,
                    wheel_Ka,
                    wheel_Ke,
                    mus,
                    CombinedPacejkasMagicFormula<float>{
                        .f = FixedArray<PacejkasMagicFormulaArgmax<float>, 2>{
                            PacejkasMagicFormulaArgmax<float>{PacejkasMagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.longitudinal_friction_steepness}},
                            PacejkasMagicFormulaArgmax<float>{PacejkasMagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.lateral_friction_steepness}}
                        }
                    },
                    vehicle_mount_0,
                    vehicle_mount_1,
                    wheel_radius);
            };
        create_wheel(
            0,
            engine,
            std::nullopt,
            wheel_mount_0,
            wheel_mount_1);
            
        rb.drivers_.set_seats({ "driver" });

        {
            if (rb.animation_state_updater_ != nullptr) {
                THROW_OR_ABORT("Rigid body already has a style updater");
            }
            auto updater = std::make_unique<AvatarAnimationUpdater>(
                rb,
                gun_node,
                args.at<std::string>(KnownArgs::animation_resource_wo_gun),
                args.at<std::string>(KnownArgs::animation_resource_w_gun));
            AnimationStateUpdater& ref = *updater;
            parent_node->set_animation_state_updater(std::move(updater));
            rb.animation_state_updater_ = { ref, CURRENT_SOURCE_LOCATION };
        }

        scene.get_node(human_head_node, DP_LOC)
            ->set_bone(SceneNodeBone{
                .name = VH{"head"},
                .smoothness = 0.9f,
                .rotation_strength = 0.2f});

        scene.get_node(main_gun_node_visual0, DP_LOC)
            ->set_bone(SceneNodeBone{
                .name = VH{"hand_r"},
                .smoothness = 0,
                .rotation_strength = 1.f});

        if (args.at<bool>(KnownArgs::if_damageable)) {
            create_damageable.execute(JsonView{std::map<std::string, nlohmann::json>{
                {"node", parent},
                {"health", 200},
                {"delete_node_when_health_leq_zero", false},
                {"explosions", std::vector<nlohmann::json>{
                    std::map<std::string, nlohmann::json>{
                        {"damage_sources", {"crash"}},
                        {"audio", "impactcrunch01"}
                    }
                }}
            }});
            auto d = std::make_unique<Crash>(rb, 0.3f);
            rb.collision_observers_.emplace_back(std::move(d));
        }
    }
    if (if_with_graphics) {
        child_renderable_instance("human-inst", animation_node, VH{asset_id + decimate});
        child_renderable_instance("human-inst" + decimate1, animation_node, VH{asset_id + decimate1});
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_generic_avatar",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateGenericAvatar(args.physics_scene(), args.macro_line_executor).execute(args.arguments);
            });
    }
} obj;

}
