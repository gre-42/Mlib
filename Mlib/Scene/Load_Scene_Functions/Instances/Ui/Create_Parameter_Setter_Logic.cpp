#include "Create_Parameter_Setter_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Strings/Trim.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(icon);
DECLARE_ARGUMENT(required);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(deflt);
DECLARE_ARGUMENT(on_change);
DECLARE_ARGUMENT(assets);
DECLARE_ARGUMENT(asset_prefix);
DECLARE_ARGUMENT(parameters);
}

const std::string CreateParameterSetterLogic::key = "parameter_setter";

LoadSceneJsonUserFunction CreateParameterSetterLogic::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateParameterSetterLogic(args.renderable_scene()).execute(args);
};

CreateParameterSetterLogic::CreateParameterSetterLogic(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateParameterSetterLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto id = args.arguments.at<std::string>(KnownArgs::id);
    std::list<ReplacementParameter> rps;
    if (args.arguments.contains(KnownArgs::parameters)) {
        rps = args.arguments.at<std::list<ReplacementParameter>>(KnownArgs::parameters);
    }
    if (args.arguments.contains(KnownArgs::assets)) {
        auto& assets = args.asset_references.get_replacement_parameters(args.arguments.at<std::string>(KnownArgs::assets));
        for (const auto& [_, a] : assets) {
            auto& rp = rps.emplace_back(ReplacementParameter{
                .title = a.title,
                .required = a.required});
            rp.globals.merge(a.globals, args.arguments.at<std::string>(KnownArgs::asset_prefix, ""));
        }
    }
    args.ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title = args.arguments.at<std::string>(KnownArgs::title),
            .icon = args.arguments.at<std::string>(KnownArgs::icon),
            .requires_ = args.arguments.at<std::vector<std::string>>(KnownArgs::required, std::vector<std::string>{})
        },
        args.arguments.at<size_t>(KnownArgs::deflt));
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,   // read by ParameterSetterLogic
        .particle_resources = primary_rendering_context.particle_resources,     // read by ParameterSetterLogic
        .rendering_resources = primary_rendering_context.rendering_resources,     // read by ParameterSetterLogic
        .z_order = 1} };                                                          // read by render_logics
    auto parameter_setter_logic = std::make_shared<ParameterSetterLogic>(
        "",
        std::vector<ReplacementParameter>{rps.begin(), rps.end()},
        args.arguments.path(KnownArgs::ttf_file),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } },
        args.external_json_macro_arguments,
        button_press,
        args.ui_focus.selection_ids.at(id),
        [mle=args.macro_line_executor, on_change=args.arguments.at<std::vector<nlohmann::json>>(KnownArgs::on_change, std::vector<nlohmann::json>{})]() {
            for (const auto& j : on_change) {
                mle(JsonView{j}, nullptr, nullptr);
            }
        });
    render_logics.append(nullptr, parameter_setter_logic);
}
