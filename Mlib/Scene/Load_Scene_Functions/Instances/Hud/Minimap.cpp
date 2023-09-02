#include "Minimap.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Minimap_Logic.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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

const std::string Minimap::key = "minimap";

LoadSceneJsonUserFunction Minimap::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    Minimap(args.renderable_scene()).execute(args);
};

Minimap::Minimap(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void Minimap::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& player = players.get_player(args.arguments.at<std::string>(KnownArgs::player));
    DanglingRef<SceneNode> node = player.scene_node();
    auto widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top)));
    auto c = std::make_shared<MinimapLogic>(
        node,
        args.arguments.path_or_variable(KnownArgs::minimap).path,
        args.arguments.path_or_variable(KnownArgs::locator).path,
        std::move(widget),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::locator_size)),
        args.arguments.at<float>(KnownArgs::pointer_reference_length),
        args.arguments.at<float>(KnownArgs::pointer_scale),
        args.arguments.at<FixedArray<float, 2>>(KnownArgs::pointer_size),
        args.arguments.at<FixedArray<double, 2>>(KnownArgs::pointer_offset));
    physics_engine.advance_times_.add_advance_time(*c);
    player.append_delete_externals(
        nullptr,
        [&at=physics_engine.advance_times_, &rl=render_logics, l=c.get()]()
        {
            at.delete_advance_time(*l, CURRENT_SOURCE_LOCATION);
            rl.remove(*l);
        }
    );
    render_logics.append(nullptr, c);
}
