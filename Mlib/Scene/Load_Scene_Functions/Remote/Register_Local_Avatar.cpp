#include "Register_Local_Avatar.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Remote/Avatar_Parameters.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

RegisterLocalAvatar::RegisterLocalAvatar(
    PhysicsScene& physics_scene,
    const MacroLineExecutor& macro_line_executor) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene, &macro_line_executor }
{}

void RegisterLocalAvatar::execute(const JsonView& args) {
    args.validate(AvatarParameters::options);
    if (remote_scene == nullptr) {
        throw std::runtime_error("Remote scene is null");
    }
    auto suffix = args.at<std::string>(AvatarParameters::suffix);
    auto name = VariableAndHash<std::string>{"human_node" + suffix};
    auto rb = get_rigid_body_vehicle(
        scene.get_node(name, CURRENT_SOURCE_LOCATION).get(),
        CURRENT_SOURCE_LOCATION);
    rb->remote_object_id_ = remote_scene->create_local<RemoteRigidBodyVehicle>(
        CURRENT_SOURCE_LOCATION,
        RemoteSceneObjectType::RIGID_BODY_AVATAR,
        RemoteObjectId{remote_scene->local_site_id(), remote_scene->next_local_object_id()},
        args.json(),
        suffix,
        rb.set_loc(CURRENT_SOURCE_LOCATION),
        DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION});
    rb->owner_site_id_ = remote_scene->local_site_id();
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
