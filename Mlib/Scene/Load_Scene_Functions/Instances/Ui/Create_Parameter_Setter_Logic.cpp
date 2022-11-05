#include "Create_Parameter_Setter_Logic.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(TITLE);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(DEFAULT);
DECLARE_OPTION(ON_FIRST_RENDER);
DECLARE_OPTION(ON_CHANGE);
DECLARE_OPTION(PARAMETERS);

LoadSceneUserFunction CreateParameterSetterLogic::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*parameter_setter"
        "\\s+id=([\\w+-.]+),"
        "\\s+title=([\\w+-. ]*),"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+),"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+),"
        "(?:\\s+size=([\\w+-.]+)\\s+([\\w+-.]+),)?"
        "\\s+font_height=([\\w+-.]+),"
        "\\s+line_distance=([\\w+-.]+),"
        "\\s+default=([\\d]+),"
        "\\s+on_first_render=([^,]*),"
        "\\s+on_change=([^,]*),"
        "\\s+parameters=([\\s\\S]*)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateParameterSetterLogic(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateParameterSetterLogic::CreateParameterSetterLogic(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateParameterSetterLogic::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string id = match[ID].str();
    std::list<ReplacementParameter> rps;
    for (const auto& e : find_all_name_values(match[PARAMETERS].str(), "[\\w+-. %]+", substitute_pattern)) {
        rps.push_back(ReplacementParameter{
            .name = e.first,
            .substitutions = SubstitutionMap{ replacements_to_map(e.second) } });
    }
    args.ui_focus.insert_submenu(
        id,
        match[TITLE].str(),
        safe_stoz(match[DEFAULT].str()));
    auto parameter_setter_logic = std::make_shared<ParameterSetterLogic>(
        "",
        std::vector<ReplacementParameter>{rps.begin(), rps.end()},
        args.fpath(match[TTF_FILE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        FixedArray<float, 2>{
            match[SIZE_X].matched ? safe_stof(match[SIZE_X].str()) : NAN,
            match[SIZE_Y].matched ? safe_stof(match[SIZE_Y].str()) : NAN},
        safe_stof(match[FONT_HEIGHT].str()),
        safe_stof(match[LINE_DISTANCE].str()),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } },
        args.external_substitutions,
        button_press,
        args.ui_focus.selection_ids.at(id),
        [mle=args.macro_line_executor, on_first_render=match[ON_FIRST_RENDER].str(), &rsc=args.rsc]() {
            if (!on_first_render.empty()) {
                mle(on_first_render, nullptr, rsc);
            }
        },
        [mle=args.macro_line_executor, on_change=match[ON_CHANGE].str(), &rsc=args.rsc]() {
            if (!on_change.empty()) {
                mle(on_change, nullptr, rsc);
            }
        });
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = secondary_rendering_context.scene_node_resources,
        .rendering_resources = secondary_rendering_context.rendering_resources,
        .z_order = 1} };
    render_logics.append(nullptr, parameter_setter_logic);
}
