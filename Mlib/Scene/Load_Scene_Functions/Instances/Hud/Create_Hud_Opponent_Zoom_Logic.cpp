#include "Create_Hud_Opponent_Zoom_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Render_Logics/Camera_Stream_Logic.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Opponent_Zoom_Logic.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(exclusive_node);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(fov);
DECLARE_ARGUMENT(zoom);
DECLARE_ARGUMENT(background_color);
}

const std::string CreateHudOpponentZoom::key = "hud_opponent_zoom";

LoadSceneJsonUserFunction CreateHudOpponentZoom::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateHudOpponentZoom(args.renderable_scene()).execute(args);
};

CreateHudOpponentZoom::CreateHudOpponentZoom(RenderableScene& renderable_scene) 
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateHudOpponentZoom::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingPtr<SceneNode> exclusive_node = nullptr;
    if (args.arguments.contains_non_null(KnownArgs::exclusive_node)) {
        exclusive_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::exclusive_node), DP_LOC).ptr();
    }
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto cam_stream = std::make_unique<CameraStreamLogic>(
        scene,
        selected_cameras,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::background_color),
        ClearMode::DEPTH);
    object_pool.create<HudOpponentZoomLogic>(
        CURRENT_SOURCE_LOCATION,
        object_pool,
        scene,
        std::move(cam_stream),
        render_logics,
        players,
        player,
        exclusive_node,
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.arguments.at<float>(KnownArgs::fov) * degrees,
        args.arguments.at<float>(KnownArgs::zoom));
}
