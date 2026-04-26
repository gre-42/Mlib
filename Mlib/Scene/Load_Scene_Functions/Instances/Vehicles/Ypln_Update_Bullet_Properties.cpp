#include "Ypln_Update_Bullet_Properties.hpp"
#include <Mlib/Components/Aim_At.hpp>
#include <Mlib/Components/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(gun_node);
DECLARE_ARGUMENT(ypln_node);
DECLARE_ARGUMENT(bullet_type);
DECLARE_ARGUMENT(dpitch_head);
}

YplnUpdateBulletProperties::YplnUpdateBulletProperties(PhysicsScene& physics_scene)
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void YplnUpdateBulletProperties::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingBaseClassRef<SceneNode> gun_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::gun_node), CURRENT_SOURCE_LOCATION);
    DanglingBaseClassRef<SceneNode> ypln_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::ypln_node), CURRENT_SOURCE_LOCATION);
    auto aim_at = get_aim_at(gun_node.get(), CURRENT_SOURCE_LOCATION);
    auto ypln = get_yaw_pitch_look_at_nodes(ypln_node.get(), CURRENT_SOURCE_LOCATION);

    const auto& bullet_type = args.bullet_property_db.get(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::bullet_type));
    aim_at->set_bullet_velocity(bullet_type.velocity);
    aim_at->set_bullet_feels_gravity(!any(bullet_type.rigid_body_flags & RigidBodyVehicleFlags::FEELS_NO_GRAVITY));
    ypln->pitch_look_at_node()->set_dpitch_head(args.arguments.at<float>(KnownArgs::dpitch_head) * degrees);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "ypln_update_bullet_properties",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                YplnUpdateBulletProperties(args.physics_scene()).execute(args);            
            });
    }
} obj;

}
