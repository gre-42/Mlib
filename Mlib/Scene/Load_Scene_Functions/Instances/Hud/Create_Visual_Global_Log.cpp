#include "Create_Visual_Global_Log.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Global_Log.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Log_Entry_Severity.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(NENTRIES);
DECLARE_OPTION(SEVERITY);

LoadSceneUserFunction CreateVisualGlobalLog::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*visual_global_log"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+nentries=([\\d+]+)"
        "\\s+severity=(info|critical)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateVisualGlobalLog(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateVisualGlobalLog::CreateVisualGlobalLog(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateVisualGlobalLog::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto logger = std::make_shared<VisualGlobalLog>(
        base_log,
        args.fpath(match[TTF_FILE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        safe_stof(match[FONT_HEIGHT].str()),
        safe_stof(match[LINE_DISTANCE].str()),
        safe_stoz(match[NENTRIES].str()),
        log_entry_severity_from_string(match[SEVERITY].str()));
    render_logics.append(nullptr, logger);
}
