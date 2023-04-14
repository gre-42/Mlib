#include "Create_Parameter_Setter_Logic.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Strings/Trim.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(TITLE);
DECLARE_OPTION(ICON);
DECLARE_OPTION(REQUIRES);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(DEFAULT);
DECLARE_OPTION(ON_CHANGE);
DECLARE_OPTION(ASSETS);
DECLARE_OPTION(ASSET_PREFIX);
DECLARE_OPTION(PARAMETERS);

const std::string CreateParameterSetterLogic::key = "parameter_setter";

LoadSceneUserFunction CreateParameterSetterLogic::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^id=([\\w+-.]+)"
        ",\\s+title=([\\w+-. ]*)"
        ",\\s+icon=(\\w+)"
        "(?:,\\s+requires=([^,]*))?"
        ",\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        ",\\s+left=(\\w+)"
        ",\\s+right=(\\w+)"
        ",\\s+bottom=(\\w+)"
        ",\\s+top=(\\w+)"
        ",\\s+font_height=(\\w+)"
        ",\\s+line_distance=(\\w+)"
        ",\\s+default=([\\d]+)"
        ",\\s+on_change=([^,]*)"
        "(?:,\\s+assets=([^,]+))?"
        "(?:,\\s+asset_prefix=([^,]+))?"
        "(?:,\\s+parameters=([\\s\\S]*))?$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateParameterSetterLogic(args.renderable_scene()).execute(match, args);
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
            .variables = SubstitutionMap{ replacements_to_map(e.second) } });
    }
    if (match[ASSETS].matched) {
        auto assets = args.asset_references.get_replacement_parameters(match[ASSETS].str());
        for (const auto& a : assets) {
            rps.push_back(ReplacementParameter{
                .name = a.name,
                .on_init = a.on_init,
                .requires_ = a.requires_});
            rps.back().variables.merge(a.variables, match[ASSET_PREFIX].str());
        }
    }
    args.ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title = match[TITLE].str(),
            .icon = match[ICON].str(),
            .requires_ = string_to_vector(match[REQUIRES].str(), [](const std::string& v){return rtrim_copy(v, ':');})
        },
        safe_stoz(match[DEFAULT].str()));
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,   // read by ParameterSetterLogic
        .rendering_resources = primary_rendering_context.rendering_resources,     // read by ParameterSetterLogic
        .z_order = 1} };                                                          // read by render_logics
    auto parameter_setter_logic = std::make_shared<ParameterSetterLogic>(
        "",
        std::vector<ReplacementParameter>{rps.begin(), rps.end()},
        args.fpath(match[TTF_FILE].str()).path,
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(match[LEFT].str()),
            args.layout_constraints.get_pixels(match[RIGHT].str()),
            args.layout_constraints.get_pixels(match[BOTTOM].str()),
            args.layout_constraints.get_pixels(match[TOP].str())),
        args.layout_constraints.get_pixels(match[FONT_HEIGHT].str()),
        args.layout_constraints.get_pixels(match[LINE_DISTANCE].str()),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } },
        args.external_substitutions,
        button_press,
        args.ui_focus.selection_ids.at(id),
        [mle=args.macro_line_executor, on_change=match[ON_CHANGE].str()]() {
            if (!on_change.empty()) {
                mle(on_change, nullptr);
            }
        });
    render_logics.append(nullptr, parameter_setter_logic);
}
