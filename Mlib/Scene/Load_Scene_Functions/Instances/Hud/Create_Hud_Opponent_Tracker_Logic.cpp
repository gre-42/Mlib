#include "Create_Hud_Opponent_Tracker_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Opponent_Tracker_Logic.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(exclusive_node);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(center);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(error_behavior);
}

const std::string CreateHudOpponentTracker::key = "hud_opponent_tracker";

LoadSceneJsonUserFunction CreateHudOpponentTracker::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateHudOpponentTracker(args.renderable_scene()).execute(args);
};

CreateHudOpponentTracker::CreateHudOpponentTracker(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateHudOpponentTracker::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingPtr<SceneNode> exclusive_node = nullptr;
    if (args.arguments.contains_non_null(KnownArgs::exclusive_node)) {
        exclusive_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::exclusive_node), DP_LOC).ptr();
    }
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    object_pool.create<HudOpponentTrackerLogic>(
        CURRENT_SOURCE_LOCATION,
        object_pool,
        scene_logic,
        render_logics,
        players,
        player,
        exclusive_node,
        physics_engine.advance_times_,
        RenderingContextStack::primary_rendering_resources().get_texture_lazy(
            ColormapWithModifiers{
                .filename = VariableAndHash{ args.arguments.path(KnownArgs::filename) },
                .color_mode = ColorMode::RGBA
            }.compute_hash(),
            TextureRole::COLOR_FROM_DB),
        args.arguments.at<EFixedArray<float, 2>>(KnownArgs::center),
        args.arguments.at<EFixedArray<float, 2>>(KnownArgs::size),
        hud_error_behavior_from_string(args.arguments.at<std::string>(KnownArgs::error_behavior)));
}
