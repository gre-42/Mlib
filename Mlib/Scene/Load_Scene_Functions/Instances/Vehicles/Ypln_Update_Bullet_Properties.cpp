#include "Ypln_Update_Bullet_Properties.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(feels_gravity);
DECLARE_ARGUMENT(dpitch_head);
}

const std::string YplnUpdateBulletProperties::key = "ypln_update_bullet_properties";

LoadSceneUserFunction YplnUpdateBulletProperties::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments{nlohmann::json::parse(args.line)};
    json_macro_arguments.validate(KnownArgs::options);
    YplnUpdateBulletProperties(args.renderable_scene()).execute(json_macro_arguments, args);
};

YplnUpdateBulletProperties::YplnUpdateBulletProperties(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void YplnUpdateBulletProperties::execute(
    const JsonMacroArguments& json_macro_arguments,
    const LoadSceneUserFunctionArgs& args)
{
    auto& ypln_node = scene.get_node(json_macro_arguments.at<std::string>(KnownArgs::node));
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&ypln_node.get_relative_movable());
    if (ypln == nullptr) {
        THROW_OR_ABORT("Relative movable is not a ypln");
    }
    ypln->set_bullet_velocity(json_macro_arguments.at<float>(KnownArgs::velocity) * meters / s);
    ypln->set_bullet_feels_gravity(json_macro_arguments.at<bool>(KnownArgs::feels_gravity));
    ypln->pitch_look_at_node().set_dpitch_head(json_macro_arguments.at<float>(KnownArgs::dpitch_head) * degrees);
}
