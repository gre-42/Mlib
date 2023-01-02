#include "Create_Visual_Player_Bullet_Count.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Bullet_Count.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);

LoadSceneUserFunction CreateVisualPlayerBulletCount::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*visual_player_bullet_count"
        "\\s+player=([\\w+-.]+)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateVisualPlayerBulletCount(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateVisualPlayerBulletCount::CreateVisualPlayerBulletCount(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateVisualPlayerBulletCount::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& player = players.get_player(match[PLAYER].str());
    auto logger = std::make_shared<VisualBulletCount>(
        physics_engine.advance_times_,
        player,
        args.fpath(match[TTF_FILE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        FixedArray<float, 2>{
            safe_stof(match[SIZE_X].str()),
            safe_stof(match[SIZE_Y].str())},
        safe_stof(match[FONT_HEIGHT].str()),
        safe_stof(match[LINE_DISTANCE].str()));
    physics_engine.advance_times_.add_advance_time(logger);
    player.append_delete_externals(
        nullptr,
        [&at=physics_engine.advance_times_, &rl=render_logics, l=logger.get()]()
        {
            at.schedule_delete_advance_time(*l);
            rl.remove(*l);
        }
    );
    render_logics.append(nullptr, logger);
}
