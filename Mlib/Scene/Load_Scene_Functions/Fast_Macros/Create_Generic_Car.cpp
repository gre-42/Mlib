#include "Create_Generic_Car.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Wheel.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Car_Controller.cpp>
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Child_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Child_Renderable_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rigid_Cuboid.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rigid_Disk.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(NAME);
DECLARE_ARGUMENT(TESUFFIX);
DECLARE_ARGUMENT(DECIMATE);
DECLARE_ARGUMENT(IF_WITH_GRAPHICS);
DECLARE_ARGUMENT(IF_WITH_PHYSICS);
DECLARE_ARGUMENT(HAND_BRAKE_PULLED);
DECLARE_ARGUMENT(VELOCITY);
DECLARE_ARGUMENT(ANGULAR_VELOCITY);
}

namespace KnownDb {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(max_tire_angle);
DECLARE_ARGUMENT(tire_angle_velocities);
DECLARE_ARGUMENT(tire_angles);
DECLARE_ARGUMENT(light_left_front_position);
DECLARE_ARGUMENT(light_right_front_position);
DECLARE_ARGUMENT(wheel_left_front_mount_0);
DECLARE_ARGUMENT(wheel_left_front_mount_1);
DECLARE_ARGUMENT(wheel_right_front_mount_0);
DECLARE_ARGUMENT(wheel_right_front_mount_1);
DECLARE_ARGUMENT(wheel_left_rear_mount_0);
DECLARE_ARGUMENT(wheel_left_rear_mount_1);
DECLARE_ARGUMENT(wheel_right_rear_mount_0);
DECLARE_ARGUMENT(wheel_right_rear_mount_1);
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
DECLARE_ARGUMENT(wheels);
DECLARE_ARGUMENT(mute);
DECLARE_ARGUMENT(w_clutch);
DECLARE_ARGUMENT(max_dw);
DECLARE_ARGUMENT(front_engine);
DECLARE_ARGUMENT(rear_engine);
DECLARE_ARGUMENT(audio);
}

namespace KnownAudio {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(p_idle);
DECLARE_ARGUMENT(p_reference);
DECLARE_ARGUMENT(mute);
}

namespace KnownWheels {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(mass);
DECLARE_ARGUMENT(musF);
DECLARE_ARGUMENT(musC);
DECLARE_ARGUMENT(brake_force);
DECLARE_ARGUMENT(brake_torque);
DECLARE_ARGUMENT(Ks);
DECLARE_ARGUMENT(Ka);

}

static inline float stow(float v) {
    return v * rpm;
}

static inline float stop(float v) {
    return v * hp;
}

static inline float stov(float v) {
    return v * kph;
}

static inline float stoa(float v) {
    return v * degrees;
}

const std::string CreateGenericCar::key = "create_generic_car";

LoadSceneJsonUserFunction CreateGenericCar::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateGenericCar(args.renderable_scene()).execute(args);
};

CreateGenericCar::CreateGenericCar(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateGenericCar::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto create_child_node = CreateChildNode{ renderable_scene };
    auto child_renderable_instance = ChildRenderableInstance{ renderable_scene };
    auto create_rigid_cuboid = CreateRigidCuboid{ renderable_scene };
    auto create_rigid_disk = CreateRigidDisk{ renderable_scene };

    auto NAME = args.arguments.at<std::string>(KnownArgs::NAME);
    auto TESUFFIX = args.arguments.at<std::string>(KnownArgs::TESUFFIX);
    auto DECIMATE = args.arguments.at<std::string>(KnownArgs::DECIMATE);
    auto IF_WITH_GRAPHICS = args.arguments.at<bool>(KnownArgs::IF_WITH_GRAPHICS);
    auto IF_WITH_PHYSICS = args.arguments.at<bool>(KnownArgs::IF_WITH_PHYSICS);
    const auto& vdb = args.asset_references["vehicles"].at(NAME).rp.database;
    auto wheels = vdb.at<std::string>(KnownDb::wheels);
    const auto& wdb = args.asset_references["wheels"].at(wheels).rp.database;
    auto parent = "car_node" + TESUFFIX;

    auto wheel_left_front_mount_0 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_left_front_mount_0);
    auto wheel_left_front_mount_1 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_left_front_mount_1);
    auto wheel_right_front_mount_0 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_right_front_mount_0);
    auto wheel_right_front_mount_1 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_right_front_mount_1);
    auto wheel_left_rear_mount_0 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_left_rear_mount_0);
    auto wheel_left_rear_mount_1 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_left_rear_mount_1);
    auto wheel_right_rear_mount_0 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_right_rear_mount_0);
    auto wheel_right_rear_mount_1 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_right_rear_mount_1);

    create_child_node("dynamic", parent, "wheel_left_front_node" + TESUFFIX, wheel_left_front_mount_0.casted<ScenePos>());
    create_child_node("dynamic", parent, "wheel_right_front_node" + TESUFFIX, wheel_right_front_mount_0.casted<ScenePos>());
    create_child_node("dynamic", parent, "wheel_left_rear_node" + TESUFFIX, wheel_left_rear_mount_0.casted<ScenePos>());
    create_child_node("dynamic", parent, "wheel_right_rear_node" + TESUFFIX, wheel_right_rear_mount_0.casted<ScenePos>());

    if (IF_WITH_GRAPHICS) {
        create_child_node("dynamic", "wheel_right_front_node" + TESUFFIX, "wheel_right_front_node_visual" + TESUFFIX, { 0.f, 0.f, 0.f }, { 0.f, 180 * degrees, 0.f });
        create_child_node("dynamic", "wheel_right_rear_node" + TESUFFIX, "wheel_right_rear_node_visual" + TESUFFIX, { 0.f, 0.f, 0.f }, { 0.f, 180 * degrees, 0.f });
        auto create_graphics = [&](const std::string& suffix, const std::string& decimate){
            child_renderable_instance("main" + suffix, parent, NAME + "/main" + decimate);

            child_renderable_instance("wheel" + suffix, "wheel_left_front_node" + TESUFFIX, NAME + "/wheel_front" + decimate);
            child_renderable_instance("wheel" + suffix, "wheel_right_front_node_visual" + TESUFFIX, NAME + "/wheel_front" + decimate);
            child_renderable_instance("wheel" + suffix, "wheel_left_rear_node" + TESUFFIX, NAME + "/wheel_rear" + decimate);
            child_renderable_instance("wheel" + suffix, "wheel_right_rear_node_visual" + TESUFFIX, NAME + "/wheel_rear" + decimate);
            };
        create_graphics(TESUFFIX, DECIMATE);
        create_graphics("_lowres" + TESUFFIX, "_lowres");
        auto create_lights = [&]() {
            if (auto p = vdb.try_at_non_null<UFixedArray<ScenePos, 3>>(KnownDb::light_left_front_position); p.has_value()) {
                create_child_node("dynamic", parent, "light_left_front" + TESUFFIX, *p);
                child_renderable_instance("blob" + TESUFFIX, "light_left_front" + TESUFFIX, "car_light_beam");
            }
            if (auto p = vdb.try_at_non_null<UFixedArray<ScenePos, 3>>(KnownDb::light_right_front_position); p.has_value()) {
                create_child_node("dynamic", parent, "light_right_front_position" + TESUFFIX, *p);
                child_renderable_instance("blob" + TESUFFIX, "light_right_front_position" + TESUFFIX, "car_light_beam");
            }};
        create_lights();
    }
    auto create_physics = [&](){
        DanglingRef<SceneNode> node = scene.get_node(parent, DP_LOC);

        auto& rb = create_rigid_cuboid(CreateRigidCuboidArgs{
            .object_pool = object_pool,
            .node = parent,
            .name = "generic_car_" + NAME + TESUFFIX,
            .asset_id = NAME,
            .mass = vdb.at<float>(KnownDb::mass) * kg,
            .size = vdb.at<UFixedArray<float, 3>>(KnownDb::size) * meters,
            .com = vdb.at<UFixedArray<float, 3>>(KnownDb::com) * meters,
            .v = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::VELOCITY) * kph,
            .w = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::ANGULAR_VELOCITY) * rpm,
            .I_rotation = fixed_zeros<float, 3>(),
            .geographic_coordinates = scene_node_resources.get_geographic_mapping("world"),
            .waypoint_dy = (CompressedScenePos)1.2f,
            .hitboxes = NAME + "_hitboxes",
            .collidable_mode = CollidableMode::MOVING});

        std::shared_ptr<EngineAudio> av;
        if (vdb.contains(KnownDb::audio)) {
            auto a = vdb.child(KnownDb::audio);
            a.validate(KnownAudio::options);
            if (!a.at<bool>(KnownAudio::mute)) {
                av = std::make_shared<EngineAudio>(
                    a.at<std::string>(KnownAudio::name),
                    paused,
                    a.at<float>(KnownAudio::p_idle) * hp,
                    a.at<float>(KnownAudio::p_reference) * hp);
            }
        }

        auto front_engine = vdb.at<VariableAndHash<std::string>>(KnownDb::front_engine);
        auto rear_engine = vdb.at<VariableAndHash<std::string>>(KnownDb::rear_engine);
        if (front_engine != rear_engine) {
            {
                auto engine_power = EnginePower{
                    Interp<float>{
                        vdb.at_vector<float>(KnownDb::angular_vels_front, stow),
                        vdb.at_vector<float>(KnownDb::powers_front, stop),
                        OutOfRangeBehavior::CLAMP},
                    vdb.at<std::vector<float>>(KnownDb::gear_ratios),
                    vdb.at<float>(KnownDb::w_clutch) * rpm,
                    vdb.at<float>(KnownDb::max_dw, INFINITY) * rpm / seconds };
                rb.engines_.add(
                    VariableAndHash<std::string>{ "front" },
                    std::move(engine_power),
                    args.arguments.at<bool>(KnownArgs::HAND_BRAKE_PULLED, false),
                    av);
            }
            {
                auto engine_power = EnginePower{
                    Interp<float>{
                    vdb.at_vector<float>(KnownDb::angular_vels_rear, stow),
                        vdb.at_vector<float>(KnownDb::powers_rear, stop),
                        OutOfRangeBehavior::CLAMP},
                    vdb.at<std::vector<float>>(KnownDb::gear_ratios),
                    vdb.at<float>(KnownDb::w_clutch) * rpm,
                    vdb.at<float>(KnownDb::max_dw, INFINITY) * rpm / seconds };
                rb.engines_.add(
                    VariableAndHash<std::string>{ "rear" },
                    std::move(engine_power),
                    args.arguments.at<bool>(KnownArgs::HAND_BRAKE_PULLED, false),
                    av);
            }
        } else if (vdb.contains_non_null(KnownDb::powers)) {
            auto engine_power = EnginePower{
                Interp<float>{
                    vdb.at_vector<float>(KnownDb::angular_vels, stow),
                    vdb.at_vector<float>(KnownDb::powers, stop),
                    OutOfRangeBehavior::CLAMP},
                vdb.at<std::vector<float>>(KnownDb::gear_ratios),
                vdb.at<float>(KnownDb::w_clutch) * rpm,
                vdb.at<float>(KnownDb::max_dw, INFINITY) * rpm / seconds };
            rb.engines_.add(
                VariableAndHash<std::string>{ "engine" },
                std::move(engine_power),
                args.arguments.at<bool>(KnownArgs::HAND_BRAKE_PULLED, false),
                av);
        }

        wdb.validate(KnownWheels::options);
        auto wheel_radius = wdb.at<float>(KnownWheels::radius) * meters;
        auto wheel_mass = wdb.at<float>(KnownWheels::mass) * kg;
        auto wheel_brake_force = wdb.at<float>(KnownWheels::brake_force) * N;
        auto wheel_brake_torque = wdb.at<float>(KnownWheels::brake_torque) * N * meters;
        auto wheel_Ks = wdb.at<float>(KnownWheels::Ks) * N;
        auto wheel_Ka = wdb.at<float>(KnownWheels::Ka) * N / (meters / seconds);
        Interp<float> mus{
            wdb.at<std::vector<float>>(KnownWheels::musF),
            wdb.at<std::vector<float>>(KnownWheels::musC),
            OutOfRangeBehavior::CLAMP};
        auto create_wheel = [&](
            size_t tire_id,
            const std::string& node,
            const std::string& name,
            const std::string& asset_id,
            const VariableAndHash<std::string>& engine,
            const std::optional<VariableAndHash<std::string>>& delta_engine,
            const FixedArray<float, 3>& vehicle_mount_0,
            const FixedArray<float, 3>& vehicle_mount_1)
            {
                RigidBodyVehicle* wheel = nullptr;
                if (wheel_mass != 0.f) {
                    wheel = &create_rigid_disk(CreateRigidDiskArgs{
                        .object_pool = object_pool,
                        .node = node,
                        .name = name,
                        .asset_id = asset_id,
                        .mass = wheel_mass,
                        .radius = wheel_radius,
                        .com = fixed_zeros<float, 3>(),
                        .v = fixed_zeros<float, 3>(),
                        .w = fixed_zeros<float, 3>(),
                        .I_rotation = { 0.f, 90 * degrees, 0.f },
                        .geographic_coordinates = nullptr,
                        .flags = RigidBodyVehicleFlags::NONE,
                        .waypoint_dy = (CompressedScenePos)0.f,
                        .hitboxes = std::nullopt,
                        .hitbox_filter = PhysicsResourceFilter{},
                        .collidable_mode = CollidableMode::NONE });
                }

                RigidBodyPulses* wheel_rbp = nullptr;
                if (!node.empty()) {
                    auto wheel_node = scene.get_node(node, DP_LOC);
                    if (wheel != nullptr) {
                        wheel_rbp = &wheel->rbp_;
                    } else {
                        auto wheel = std::make_unique<Wheel>(
                            rb,
                            physics_engine.advance_times_,
                            tire_id,
                            wheel_radius);
                        Linker{ physics_engine.advance_times_ }.link_relative_movable<Wheel>(
                            wheel_node,
                            { *wheel, CURRENT_SOURCE_LOCATION },
                            CURRENT_SOURCE_LOCATION);
                        global_object_pool.add(std::move(wheel), CURRENT_SOURCE_LOCATION);
                    }
                }
                {
                    // From: https://www.nanolounge.de/21977/federkonstante-und-masse-bei-auto
                    // Ds = 1000 / 4 * 9.8 / 0.02 = 122500 = 1.225e5
                    // Da * 1 = 1000 / 4 * 9.8 => Da = 1e4 / 4
                    rb.tires_.add(
                        tire_id,
                        engine,
                        delta_engine,
                        wheel_rbp,
                        wheel_brake_force,
                        wheel_brake_torque,
                        wheel_Ks,
                        wheel_Ka,
                        mus,
                        CombinedMagicFormula<float>{
                            .f = FixedArray<MagicFormulaArgmax<float>, 2>{
                                MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.longitudinal_friction_steepness}},
                                MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.lateral_friction_steepness}}
                            }
                        },
                        vehicle_mount_0,
                        vehicle_mount_1,
                        wheel_radius);
                }
            };
        create_wheel(
            0,
            "wheel_left_front_node" + TESUFFIX,
            "wheel_left_front" + NAME + TESUFFIX,
            "wheel_left_front" + NAME,
            front_engine,
            std::nullopt,
            wheel_left_front_mount_0,
            wheel_left_front_mount_1);
        create_wheel(
            1,
            "wheel_right_front_node" + TESUFFIX,
            "wheel_right_front" + NAME + TESUFFIX,
            "wheel_right_front" + NAME,
            front_engine,
            std::nullopt,
            wheel_right_front_mount_0,
            wheel_right_front_mount_1);
        create_wheel(
            2,
            "wheel_left_rear_node" + TESUFFIX,
            "wheel_left_rear" + NAME + TESUFFIX,
            "wheel_left_rear" + NAME,
            rear_engine,
            std::nullopt,
            wheel_left_rear_mount_0,
            wheel_left_rear_mount_1);
        create_wheel(
            3,
            "wheel_right_rear_node" + TESUFFIX,
            "wheel_right_rear" + NAME + TESUFFIX,
            "wheel_right_rear" + NAME,
            rear_engine,
            std::nullopt,
            wheel_right_rear_mount_0,
            wheel_right_rear_mount_1);

        if (auto pos = vdb.at("trailer_hitch_position_female"); pos.type() != nlohmann::detail::value_t::null) {
            rb.trailer_hitches_.set_position_female(pos.get<UFixedArray<float, 3>>());
        }
        if (auto pos = vdb.at("trailer_hitch_position_male"); pos.type() != nlohmann::detail::value_t::null) {
            rb.trailer_hitches_.set_position_male(pos.get<UFixedArray<float, 3>>());
        }

        auto front_tire_ids = std::vector<size_t>{ 0, 1 };
        rb.vehicle_controller_ = std::make_unique<CarController>(
            rb,
            front_engine,
            rear_engine,
            front_tire_ids,
            stoa(vdb.at<float>(KnownDb::max_tire_angle)),
            Interp<float>{
                vdb.at_vector<float>(KnownDb::tire_angle_velocities, stov),
                vdb.at_vector<float>(KnownDb::tire_angles, stoa),
                OutOfRangeBehavior::CLAMP},
                physics_engine);

        rb.drivers_.set_roles({ "driver" });
        };
    if (IF_WITH_PHYSICS) {
        create_physics();
    }
}

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(CreateGenericCar::key, CreateGenericCar::json_user_function);
    }
} obj;
