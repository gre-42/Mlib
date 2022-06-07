#include "Create_Visual_Global_Log.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Global_Log.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Log_Entry_Severity.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

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
        args.fpath(match[1].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[2].str()),
            safe_stof(match[3].str())},
        safe_stof(match[4].str()),
        safe_stof(match[5].str()),
        safe_stoz(match[6].str()),
        log_entry_severity_from_string(match[7].str()));
    render_logics.append(nullptr, logger);
}
