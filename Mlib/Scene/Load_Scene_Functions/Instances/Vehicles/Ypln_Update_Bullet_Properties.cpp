#include "Ypln_Update_Bullet_Properties.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(rigid_body_flags);
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
    auto& ypln_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&ypln_node.get_relative_movable());
    if (ypln == nullptr) {
        THROW_OR_ABORT("Relative movable is not a ypln");
    }
    auto flags = rigid_body_vehicle_flags_from_string(args.arguments.at<std::string>(KnownArgs::rigid_body_flags));
    ypln->set_bullet_velocity(args.arguments.at<float>(KnownArgs::velocity) * meters / s);
    ypln->set_bullet_feels_gravity(!any(flags & RigidBodyVehicleFlags::FEELS_NO_GRAVITY));
    ypln->pitch_look_at_node().set_dpitch_head(args.arguments.at<float>(KnownArgs::dpitch_head) * degrees);
}
