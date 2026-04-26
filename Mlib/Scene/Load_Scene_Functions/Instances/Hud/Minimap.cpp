#include "Minimap.hpp"
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Render_Logics/Render_Logics.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Minimap_Logic.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(locator_size);
DECLARE_ARGUMENT(minimap);
DECLARE_ARGUMENT(locator);
DECLARE_ARGUMENT(pointer_reference_length);
DECLARE_ARGUMENT(pointer_scale);
DECLARE_ARGUMENT(pointer_size);
DECLARE_ARGUMENT(pointer_offset);
}

Minimap::Minimap(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void Minimap::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    DanglingBaseClassRef<SceneNode> node = player->scene_node();
    auto widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top)));
    object_pool.create<MinimapLogic>(
        CURRENT_SOURCE_LOCATION,
        object_pool,
        physics_engine.advance_times_,
        render_logics,
        player,
        node,
        args.arguments.path_or_variable(KnownArgs::minimap),
        args.arguments.path_or_variable(KnownArgs::locator),
        std::move(widget),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::locator_size)),
        args.arguments.at<float>(KnownArgs::pointer_reference_length),
        args.arguments.at<float>(KnownArgs::pointer_scale),
        args.arguments.at<EFixedArray<float, 2>>(KnownArgs::pointer_size),
        args.arguments.at<EFixedArray<double, 2>>(KnownArgs::pointer_offset));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "minimap",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                Minimap{args.renderable_scene()}.execute(args);
            });
    }
} obj;

}
