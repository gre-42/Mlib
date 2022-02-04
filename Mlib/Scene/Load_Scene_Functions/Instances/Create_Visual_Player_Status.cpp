#include "Create_Visual_Player_Status.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(FORMAT);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);

LoadSceneUserFunction CreateVisualPlayerStatus::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*visual_player_status"
        "\\s+player=([\\w+-.]+)"
        "\\s+format=(\\d+)"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateVisualPlayerStatus(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateVisualPlayerStatus::CreateVisualPlayerStatus(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateVisualPlayerStatus::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& player = players.get_player(match[PLAYER].str());
    auto& node = player.scene_node();
    auto mv = node.get_absolute_movable();
    auto lo = dynamic_cast<StatusWriter*>(mv);
    if (lo == nullptr) {
        throw std::runtime_error("Could not find loggable");
    }
    StatusComponents log_components = (StatusComponents)safe_stoi(match[FORMAT].str());
    auto logger = std::make_shared<VisualMovableLogger>(
        physics_engine.advance_times_,
        lo,
        log_components,
        args.fpath(match[TTF_FILE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        safe_stof(match[FONT_HEIGHT].str()),
        safe_stof(match[LINE_DISTANCE].str()));
    physics_engine.advance_times_.add_advance_time(logger);
    players.get_player(match[PLAYER].str())
    .append_delete_externals(
        [&at=physics_engine.advance_times_, &rl=render_logics, l=logger.get()](){
            at.schedule_delete_advance_time(l);
            rl.remove(*l);});
    render_logics.append(nullptr, logger);
}
