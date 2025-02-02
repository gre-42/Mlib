#include "Ypln_Update_Bullet_Properties.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Aim_At.hpp>
#include <Mlib/Components/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(gun_node);
DECLARE_ARGUMENT(ypln_node);
DECLARE_ARGUMENT(bullet_type);
DECLARE_ARGUMENT(dpitch_head);
}

const std::string YplnUpdateBulletProperties::key = "ypln_update_bullet_properties";

LoadSceneJsonUserFunction YplnUpdateBulletProperties::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    YplnUpdateBulletProperties(args.renderable_scene()).execute(args);
};

YplnUpdateBulletProperties::YplnUpdateBulletProperties(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void YplnUpdateBulletProperties::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> gun_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::gun_node), DP_LOC);
    DanglingRef<SceneNode> ypln_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::ypln_node), DP_LOC);
    auto& aim_at = get_aim_at(gun_node);
    auto& ypln = get_yaw_pitch_look_at_nodes(ypln_node);

    const auto& bullet_type = args.bullet_property_db.get(args.arguments.at<std::string>(KnownArgs::bullet_type));
    aim_at.set_bullet_velocity(bullet_type.velocity);
    aim_at.set_bullet_feels_gravity(!any(bullet_type.rigid_body_flags & RigidBodyVehicleFlags::FEELS_NO_GRAVITY));
    ypln.pitch_look_at_node().set_dpitch_head(args.arguments.at<float>(KnownArgs::dpitch_head) * degrees);
}
