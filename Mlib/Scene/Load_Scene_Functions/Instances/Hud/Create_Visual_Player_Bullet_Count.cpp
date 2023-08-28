#include "Create_Visual_Player_Bullet_Count.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Bullet_Count.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
}

const std::string CreateVisualPlayerBulletCount::key = "visual_player_bullet_count";

LoadSceneJsonUserFunction CreateVisualPlayerBulletCount::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateVisualPlayerBulletCount(args.renderable_scene()).execute(args);
};

CreateVisualPlayerBulletCount::CreateVisualPlayerBulletCount(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateVisualPlayerBulletCount::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& player = players.get_player(args.arguments.at<std::string>(KnownArgs::player));
    auto logger = std::make_shared<VisualBulletCount>(
        physics_engine.advance_times_,
        player,
        args.arguments.path(KnownArgs::ttf_file),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)));
    physics_engine.advance_times_.add_advance_time(*logger);
    player.append_delete_externals(
        nullptr,
        [&at=physics_engine.advance_times_, &rl=render_logics, l=logger.get()]()
        {
            at.delete_advance_time(*l, std::source_location::current());
            rl.remove(*l);
        }
    );
    render_logics.append(nullptr, logger);
}
