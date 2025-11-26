#include "Register_Local_Avatar.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

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

RegisterLocalAvatar::RegisterLocalAvatar(
    PhysicsScene& physics_scene,
    const MacroLineExecutor& macro_line_executor) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene, &macro_line_executor }
{}

void RegisterLocalAvatar::execute(const JsonView& args) {
    args.validate(KnownArgs::options);
    if (remote_scene == nullptr) {
        THROW_OR_ABORT("Remote scene is null");
    }
    auto suffix = args.at<std::string>(KnownArgs::suffix);
    auto name = VariableAndHash<std::string>{"human_node" + suffix};
    auto& rb = get_rigid_body_vehicle(scene.get_node(name, CURRENT_SOURCE_LOCATION));
    rb.remote_object_id_ = remote_scene->create_local<RemoteRigidBodyVehicle>(
        CURRENT_SOURCE_LOCATION,
        RemoteSceneObjectType::RIGID_BODY_AVATAR,
        args.json().dump(),
        suffix,
        DanglingBaseClassRef<RigidBodyVehicle>{rb, CURRENT_SOURCE_LOCATION},
        DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION});
    rb.owner_site_id_ = remote_scene->local_site_id();
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "register_local_avatar",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                RegisterLocalAvatar(args.physics_scene(), args.macro_line_executor).execute(args.arguments);
            });
    }
} obj;

}
