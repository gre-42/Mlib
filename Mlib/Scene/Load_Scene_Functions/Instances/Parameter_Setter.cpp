#include "Parameter_Setter.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction ParameterSetter::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*parameter_setter"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+default=([\\d]+)"
        "\\s+on_init=([\\w+-.:= ]*)"
        "\\s+on_change=([\\w+-.:= ]*)"
        "\\s+parameters=([\\s\\S]*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        ParameterSetter(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

ParameterSetter::ParameterSetter(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ParameterSetter::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string id = match[1].str();
    std::string title = match[2].str();
    std::string ttf_filename = args.fpath(match[3].str()).path;
    FixedArray<float, 2> position{
        safe_stof(match[4].str()),
        safe_stof(match[5].str()) };
    float font_height_pixels = safe_stof(match[6].str());
    float line_distance_pixels = safe_stof(match[7].str());
    size_t deflt = safe_stoz(match[8].str());
    std::string on_init = match[9].str();
    std::string on_change = match[10].str();
    std::string parameters = match[11].str();
    std::list<ReplacementParameter> rps;
    for (const auto& e : find_all_name_values(parameters, "[\\w+-. %]+", substitute_pattern)) {
        rps.push_back(ReplacementParameter{
            .name = e.first,
            .substitutions = SubstitutionMap{ replacements_to_map(e.second) } });
    }
    args.ui_focus.insert_submenu(id, title, deflt);
    auto parameter_setter_logic = std::make_shared<ParameterSetterLogic>(
        "",
        std::vector<ReplacementParameter>{rps.begin(), rps.end()},
        ttf_filename,
        position,
        font_height_pixels,
        line_distance_pixels,        // line_distance_pixels
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_id = id },
        args.external_substitutions,
        button_press,
        args.ui_focus.selection_ids.at(id),
        [mle=args.macro_line_executor, on_change, &rsc=args.rsc]() {
            if (!on_change.empty()) {
                mle(on_change, nullptr, rsc);
            }
        });
    if (!on_init.empty()) {
        args.macro_line_executor(on_init, args.local_substitutions, args.rsc);
    }
    RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
    render_logics.append(nullptr, parameter_setter_logic);
}
