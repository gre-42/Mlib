#include "Create_Hud_Target_Point_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Target_Point_Logic.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(gun_node);
DECLARE_ARGUMENT(exclusive_node);
DECLARE_ARGUMENT(ypln_node);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(update);
DECLARE_ARGUMENT(center);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(error_behavior);
}

const std::string CreateHudTargetPointLogic::key = "hud_target_point";

LoadSceneJsonUserFunction CreateHudTargetPointLogic::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateHudTargetPointLogic(args.renderable_scene()).execute(args);
};

CreateHudTargetPointLogic::CreateHudTargetPointLogic(RenderableScene& renderable_scene) 
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateHudTargetPointLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> gun_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::gun_node), DP_LOC);
    DanglingPtr<SceneNode> exclusive_node = nullptr;
    if (args.arguments.contains_non_null(KnownArgs::exclusive_node)) {
        exclusive_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::exclusive_node), DP_LOC).ptr();
    }
    YawPitchLookAtNodes* ypln = nullptr;
    if (args.arguments.contains(KnownArgs::ypln_node)) {
        ypln = dynamic_cast<YawPitchLookAtNodes*>(&scene.get_node(args.arguments.at(KnownArgs::ypln_node), DP_LOC)->get_relative_movable());
        if (ypln == nullptr) {
            THROW_OR_ABORT("Relative movable is not a ypln");
        }
    }
    auto& player = players.get_player(args.arguments.at<std::string>(KnownArgs::player));
    auto hpl = std::make_shared<HudTargetPointLogic>(
        scene_logic,
        render_logics,
        player,
        physics_engine.collision_query_,
        gun_node,
        exclusive_node,
        ypln,
        physics_engine.advance_times_,
        args.arguments.path(KnownArgs::filename),
        resource_update_cycle_from_string(args.arguments.at(KnownArgs::update)),
        args.arguments.at<FixedArray<float, 2>>(KnownArgs::center),
        args.arguments.at<FixedArray<float, 2>>(KnownArgs::size),
        hud_error_behavior_from_string(args.arguments.at<std::string>(KnownArgs::error_behavior)));
    hpl->init();
}
