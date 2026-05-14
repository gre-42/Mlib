#include "Create_Hud_Opponent_Tracker_Logic.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Misc/FPath.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Hud_Opponent_Tracker_Logic.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(exclusive_nodes);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(center);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(error_behavior);
}

#ifdef WITHOUT_GRAPHICS
CreatePhysicsOpponentTracker::CreateHudOpponentTracker(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}
#else
CreateHudOpponentTracker::CreateHudOpponentTracker(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}
#endif

#ifdef WITHOUT_GRAPHICS
void CreatePhysicsOpponentTracker::execute(const LoadSceneJsonUserFunctionArgs& args) {
#else
void CreateHudOpponentTracker::execute(const LoadSceneJsonUserFunctionArgs& args) {
#endif
    args.arguments.validate(KnownArgs::options);
    std::optional<std::vector<DanglingBaseClassPtr<const SceneNode>>> exclusive_nodes;
    if (args.arguments.contains_non_null(KnownArgs::exclusive_nodes)) {
        exclusive_nodes = args.arguments.at_vector<VariableAndHash<std::string>>(
            KnownArgs::exclusive_nodes,
            [&scene=scene](const auto& n){ return (const DanglingBaseClassPtr<const SceneNode>&)scene.get_node(n, CURRENT_SOURCE_LOCATION).ptr(); });
    }
    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    object_pool.create<HudOpponentTrackerLogic>(
        CURRENT_SOURCE_LOCATION,
        object_pool,
        scene_logic,
        #ifdef WITHOUT_GRAPHICS
        render_logics,
        #endif
        player,
        exclusive_nodes,
        physics_engine.advance_times_,
        #ifdef WITHOUT_GRAPHICS
        RenderingContextStack::primary_rendering_resources().get_texture_lazy(
            ColormapWithModifiers{
                .filename = args.arguments.path_or_variable(KnownArgs::filename),
                .color_mode = ColorMode::RGBA
            }.compute_hash(),
            TextureRole::COLOR_FROM_DB),
        args.arguments.at<EFixedArray<float, 2>>(KnownArgs::center),
        args.arguments.at<EFixedArray<float, 2>>(KnownArgs::size),
        #endif
        hud_error_behavior_from_string(args.arguments.at<std::string>(KnownArgs::error_behavior)));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "hud_opponent_tracker",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateHudOpponentTracker{args.renderable_scene()}.execute(args);
            });
    }
} obj;

}
