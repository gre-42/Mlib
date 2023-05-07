#include "Hud_Image.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Image_Logic.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(gun_node);
DECLARE_ARGUMENT(camera_node);
DECLARE_ARGUMENT(ypln_node);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(update);
DECLARE_ARGUMENT(center);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(error_behavior);
}

const std::string HudImage::key = "hud_image";

LoadSceneJsonUserFunction HudImage::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    HudImage(args.renderable_scene()).execute(args);
};

HudImage::HudImage(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void HudImage::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto* gun_node = args.arguments.contains(KnownArgs::gun_node)
        ? &scene.get_node(args.arguments.at<std::string>(KnownArgs::gun_node))
        : nullptr;
    auto& camera_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::camera_node));
    YawPitchLookAtNodes* ypln = nullptr;
    if (args.arguments.contains(KnownArgs::ypln_node)) {
        ypln = dynamic_cast<YawPitchLookAtNodes*>(&scene.get_node(args.arguments.at(KnownArgs::ypln_node)).get_relative_movable());
        if (ypln == nullptr) {
            THROW_OR_ABORT("Relative movable is not a ypln");
        }
    }
    auto hud_image = std::make_shared<HudImageLogic>(
        &scene_logic,
        &physics_engine.collision_query_,
        gun_node,
        camera_node,
        ypln,
        physics_engine.advance_times_,
        args.arguments.path(KnownArgs::filename),
        resource_update_cycle_from_string(args.arguments.at(KnownArgs::update)),
        args.arguments.at<FixedArray<float, 2>>(KnownArgs::center),
        args.arguments.at<FixedArray<float, 2>>(KnownArgs::size),
        hud_error_behavior_from_string(args.arguments.at<std::string>(KnownArgs::error_behavior)));
    physics_engine.advance_times_.add_advance_time(*hud_image);
    camera_node.set_node_hider(*hud_image);
    render_logics.append(&camera_node, hud_image);
    players.get_player(args.arguments.at<std::string>(KnownArgs::player))
    .append_delete_externals(
        &camera_node,
        [&hi=*hud_image, &rl=render_logics](){
            rl.remove(hi);
        }
    );
}
