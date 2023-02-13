#include "Set_Externals_Creator.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(MACRO);

LoadSceneUserFunction SetExternalsCreator::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_externals_creator"
        "\\s+player=([\\w+-.]+)"
        "\\s+macro=([\\s\\S]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetExternalsCreator(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetExternalsCreator::SetExternalsCreator(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetExternalsCreator::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.get_player(match[PLAYER].str()).set_create_externals(
        [macro_line_executor = args.macro_line_executor,
         macro = match[MACRO].str()](const std::string& player_name, ExternalsMode externals_mode, const std::unordered_map<ControlSource, Skills>& skills)
        {
            if (externals_mode == ExternalsMode::NONE) {
                THROW_OR_ABORT("Invalid externals mode");
            }
            macro_line_executor(
                macro +
                "\nPLAYER_NAME:" + player_name +
                "\nIF_PC:" + ((externals_mode == ExternalsMode::PC) ? "" : "#") +
                "\nIF_MANUAL_AIM:" + (skills.at(ControlSource::USER).can_aim ? "" : "#") +
                "\nIF_MANUAL_SHOOT:" + (skills.at(ControlSource::USER).can_shoot ? "" : "#") +
                "\nIF_MANUAL_DRIVE:" + (skills.at(ControlSource::USER).can_drive ? "" : "#"),
                nullptr);
        }
    );
}
