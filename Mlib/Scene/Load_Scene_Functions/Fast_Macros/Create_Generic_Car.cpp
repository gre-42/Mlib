#include "Create_Generic_Car.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Actuators/Engine_Event_Listeners.hpp>
#include <Mlib/Physics/Actuators/Engine_Exhaust.hpp>
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Wheel.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Car_Controller.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Nodes/Create_Child_Node.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Render/Child_Renderable_Instance.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rigid_Cuboid.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Instances/Vehicles/Create_Rigid_Disk.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

using VH = VariableAndHash<std::string>;
static const auto WORLD = VariableAndHash<std::string>{"world"};

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(tesuffix);
DECLARE_ARGUMENT(tedecimate);
DECLARE_ARGUMENT(if_with_graphics);
DECLARE_ARGUMENT(if_with_physics);
DECLARE_ARGUMENT(hand_brake_pulled);
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(angular_velocity);
DECLARE_ARGUMENT(mute);
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
DECLARE_ARGUMENT(w_clutch);
DECLARE_ARGUMENT(max_dw);
DECLARE_ARGUMENT(front_engine);
DECLARE_ARGUMENT(rear_engine);
DECLARE_ARGUMENT(engine_audio);
DECLARE_ARGUMENT(engine_exhaust);
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

CreateGenericCar::CreateGenericCar(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateGenericCar::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto create_child_node = CreateChildNode{ renderable_scene };
    auto child_renderable_instance = ChildRenderableInstance{ renderable_scene };
    auto create_rigid_cuboid = CreateRigidCuboid{ renderable_scene };
    auto create_rigid_disk = CreateRigidDisk{ renderable_scene };

    auto name = args.arguments.at<std::string>(KnownArgs::name);
    auto tesuffix = args.arguments.at<std::string>(KnownArgs::tesuffix);
    auto tedecimate = args.arguments.at<std::string>(KnownArgs::tedecimate);
    auto if_with_graphics = args.arguments.at<bool>(KnownArgs::if_with_graphics);
    auto if_with_physics = args.arguments.at<bool>(KnownArgs::if_with_physics);
    const auto& vdb = args.asset_references["vehicles"].at(name).rp.database;
    auto wheels = vdb.at<std::string>(KnownDb::wheels);
    const auto& wdb = args.asset_references["wheels"].at(wheels).rp.database;
    auto parent = VH{"car_node" + tesuffix};

    auto wheel_left_front_mount_0 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_left_front_mount_0);
    auto wheel_left_front_mount_1 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_left_front_mount_1);
    auto wheel_right_front_mount_0 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_right_front_mount_0);
    auto wheel_right_front_mount_1 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_right_front_mount_1);
    auto wheel_left_rear_mount_0 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_left_rear_mount_0);
    auto wheel_left_rear_mount_1 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_left_rear_mount_1);
    auto wheel_right_rear_mount_0 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_right_rear_mount_0);
    auto wheel_right_rear_mount_1 = vdb.at<UFixedArray<float, 3>>(KnownDb::wheel_right_rear_mount_1);

    create_child_node("dynamic", parent, VH{"wheel_left_front_node" + tesuffix}, wheel_left_front_mount_0.casted<ScenePos>());
    create_child_node("dynamic", parent, VH{"wheel_right_front_node" + tesuffix}, wheel_right_front_mount_0.casted<ScenePos>());
    create_child_node("dynamic", parent, VH{"wheel_left_rear_node" + tesuffix}, wheel_left_rear_mount_0.casted<ScenePos>());
    create_child_node("dynamic", parent, VH{"wheel_right_rear_node" + tesuffix}, wheel_right_rear_mount_0.casted<ScenePos>());

    if (if_with_graphics) {
        create_child_node("dynamic", VH{"wheel_right_front_node" + tesuffix}, VH{"wheel_right_front_node_visual" + tesuffix}, { 0.f, 0.f, 0.f }, { 0.f, 180 * degrees, 0.f });
        create_child_node("dynamic", VH{"wheel_right_rear_node" + tesuffix}, VH{"wheel_right_rear_node_visual" + tesuffix}, { 0.f, 0.f, 0.f }, { 0.f, 180 * degrees, 0.f });
        auto create_graphics = [&](const std::string& suffix, const std::string& decimate){
            child_renderable_instance("main" + suffix, parent, VH{name + "/main" + decimate});

            child_renderable_instance("wheel" + suffix, VH{"wheel_left_front_node" + tesuffix}, VH{name + "/wheel_front" + decimate});
            child_renderable_instance("wheel" + suffix, VH{"wheel_right_front_node_visual" + tesuffix}, VH{name + "/wheel_front" + decimate});
            child_renderable_instance("wheel" + suffix, VH{"wheel_left_rear_node" + tesuffix}, VH{name + "/wheel_rear" + decimate});
            child_renderable_instance("wheel" + suffix, VH{"wheel_right_rear_node_visual" + tesuffix}, VH{name + "/wheel_rear" + decimate});
            };
        create_graphics(tesuffix, tedecimate);
        create_graphics("_lowres" + tesuffix, "_lowres");
        auto create_lights = [&]() {
            if (auto p = vdb.try_at_non_null<UFixedArray<ScenePos, 3>>(KnownDb::light_left_front_position); p.has_value()) {
                create_child_node("dynamic", parent, VH{"light_left_front" + tesuffix}, *p);
                child_renderable_instance("blob" + tesuffix, VH{"light_left_front" + tesuffix}, VH{"car_light_beam"});
            }
            if (auto p = vdb.try_at_non_null<UFixedArray<ScenePos, 3>>(KnownDb::light_right_front_position); p.has_value()) {
                create_child_node("dynamic", parent, VH{"light_right_front_position" + tesuffix}, *p);
                child_renderable_instance("blob" + tesuffix, VH{"light_right_front_position" + tesuffix}, VH{"car_light_beam"});
            }};
        create_lights();
    }
    auto create_physics = [&](){
        DanglingRef<SceneNode> node = scene.get_node(parent, DP_LOC);

        auto& rb = create_rigid_cuboid(CreateRigidCuboidArgs{
            .object_pool = object_pool,
            .node = parent,
            .name = "generic_car_" + name + tesuffix,
            .asset_id = name,
            .mass = vdb.at<float>(KnownDb::mass) * kg,
            .size = vdb.at<UFixedArray<float, 3>>(KnownDb::size) * meters,
            .com = vdb.at<UFixedArray<float, 3>>(KnownDb::com) * meters,
            .v = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::velocity) * kph,
            .w = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::angular_velocity) * rpm,
            .I_rotation = fixed_zeros<float, 3>(),
            .with_penetration_limits = true,
            .geographic_coordinates = scene_node_resources.get_geographic_mapping(WORLD),
            .waypoint_dy = (CompressedScenePos)1.2f,
            .hitboxes = VariableAndHash<std::string>{name + "_hitboxes"},
            .collidable_mode = CollidableMode::MOVING});

        std::shared_ptr<EngineEventListeners> engine_listeners;
        auto add_engine_listener = [&](std::shared_ptr<IEngineEventListener> l){
            if (engine_listeners == nullptr) {
                engine_listeners = std::make_shared<EngineEventListeners>();
            }
            engine_listeners->add(std::move(l));
        };

        if (!args.arguments.at<bool>(KnownArgs::mute)) {
            if (auto engine_audio = vdb.try_at_non_null<std::string>(KnownDb::engine_audio); engine_audio.has_value()) {
                const auto& adb = args.asset_references["engine_audio"].at(*engine_audio).rp.database;
                adb.validate(KnownAudio::options);
                add_engine_listener(std::make_shared<EngineAudio>(
                    adb.at<std::string>(KnownAudio::name),
                    paused,
                    paused_changed,
                    adb.at<float>(KnownAudio::p_idle) * hp,
                    adb.at<float>(KnownAudio::p_reference) * hp));
            }
        }
        if (auto engine_exhausts = vdb.try_at_non_null<std::vector<nlohmann::json>>(KnownDb::engine_exhaust); engine_exhausts.has_value()) {
            if (engine_exhausts->empty()) {
                THROW_OR_ABORT("Engine exhaust array is empty");
            }
            auto particle_renderer = std::make_shared<ParticleRenderer>(particle_resources);
            scene.get_node(parent, DP_LOC)->add_renderable(
                VariableAndHash<std::string>{"exhaust_particles"},
                particle_renderer);
            physics_engine.advance_times_.add_advance_time(
                { *particle_renderer, CURRENT_SOURCE_LOCATION },
                CURRENT_SOURCE_LOCATION);
            for (const auto& engine_exhaust : *engine_exhausts) {
                JsonView jv{ engine_exhaust };
                jv.validate(KnownExhaust::options);
                add_engine_listener(std::make_shared<EngineExhaust>(
                    RenderingContextStack::primary_rendering_resources(),
                    scene_node_resources,
                    particle_renderer,
                    scene,
                    jv.at<ConstantParticleTrail>(KnownExhaust::particle),
                    transformation_matrix_from_json<SceneDir, ScenePos, 3>(
                        jv.at(KnownExhaust::location)),
                    jv.at<float>(KnownExhaust::p_reference) * hp));
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
                    args.arguments.at<bool>(KnownArgs::hand_brake_pulled),
                    engine_listeners);
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
                    args.arguments.at<bool>(KnownArgs::hand_brake_pulled),
                    engine_listeners);
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
                args.arguments.at<bool>(KnownArgs::hand_brake_pulled),
                engine_listeners);
        } else {
            rb.engines_.add(
                VariableAndHash<std::string>{ "engine" },
                std::nullopt,   // power
                args.arguments.at<bool>(KnownArgs::hand_brake_pulled),
                nullptr);       // listeners
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
            const VariableAndHash<std::string>& node,
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
                        .with_penetration_limits = false,
                        .geographic_coordinates = nullptr,
                        .flags = RigidBodyVehicleFlags::NONE,
                        .waypoint_dy = (CompressedScenePos)0.f,
                        .hitboxes = std::nullopt,
                        .hitbox_filter = ColoredVertexArrayFilter{
                            .included_tags = PhysicsMaterial::ATTR_COLLIDE
                        },
                        .collidable_mode = CollidableMode::NONE });
                }

                RigidBodyPulses* wheel_rbp = nullptr;
                if (!node->empty()) {
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
            VariableAndHash<std::string>{"wheel_left_front_node" + tesuffix},
            "wheel_left_front" + name + tesuffix,
            "wheel_left_front" + name,
            front_engine,
            std::nullopt,
            wheel_left_front_mount_0,
            wheel_left_front_mount_1);
        create_wheel(
            1,
            VariableAndHash<std::string>{"wheel_right_front_node" + tesuffix},
            "wheel_right_front" + name + tesuffix,
            "wheel_right_front" + name,
            front_engine,
            std::nullopt,
            wheel_right_front_mount_0,
            wheel_right_front_mount_1);
        create_wheel(
            2,
            VariableAndHash<std::string>{"wheel_left_rear_node" + tesuffix},
            "wheel_left_rear" + name + tesuffix,
            "wheel_left_rear" + name,
            rear_engine,
            std::nullopt,
            wheel_left_rear_mount_0,
            wheel_left_rear_mount_1);
        create_wheel(
            3,
            VariableAndHash<std::string>{"wheel_right_rear_node" + tesuffix},
            "wheel_right_rear" + name + tesuffix,
            "wheel_right_rear" + name,
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
    if (if_with_physics) {
        create_physics();
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_generic_car",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateGenericCar(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
