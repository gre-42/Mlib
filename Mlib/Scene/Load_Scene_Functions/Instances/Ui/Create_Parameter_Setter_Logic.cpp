#include "Create_Parameter_Setter_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Strings/Trim.hpp>

using namespace Mlib;

namespace Mlib {

struct DatabaseFilter {
    std::string database;
    std::string variable;
};

static void from_json(const nlohmann::json& j, DatabaseFilter& item) {
    j.at("database").get_to(item.database);
    j.at("variable").get_to(item.variable);
}

}
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
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(deflt);
DECLARE_ARGUMENT(on_change);
DECLARE_ARGUMENT(assets);
DECLARE_ARGUMENT(database_filter);
DECLARE_ARGUMENT(hide_if_trivial);
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
        auto& assets = args.asset_references[args.arguments.at<std::string>(KnownArgs::assets)];
        for (const auto& [_, a] : assets) {
            rps.push_back(a.rp);
        }
    }
    if (args.arguments.contains(KnownArgs::database_filter)) {
        auto f = args.arguments.at<DatabaseFilter>(KnownArgs::database_filter);
        auto& assets = args.asset_references[f.database];
        std::set<std::string> ents;
        for (const auto& [_, a] : assets) {
            try {
                for (const auto& v : a.rp.database.at<std::vector<std::string>>(f.variable)) {
                    ents.insert(v);
                }
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Error processing asset \"" + a.rp.id + "\": " + e.what());
            }
        }
        rps.remove_if([&ents](const ReplacementParameter& rp) { return (!ents.contains(rp.id)); });
    }
    if (args.arguments.at<bool>(KnownArgs::hide_if_trivial, false)) {
        if (rps.empty()) {
            return;
        }
        if (rps.size() == 1) {
            const auto& rp = rps.front();
            if (!rp.on_before_select.is_null()) {
                args.macro_line_executor(rp.on_before_select, nullptr, nullptr);
            }
            return;
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
    auto& parameter_setter_logic = object_pool.create<ParameterSetterLogic>(
        CURRENT_SOURCE_LOCATION,
        "id = " + id,
        std::vector<ReplacementParameter>{rps.begin(), rps.end()},
        args.arguments.path(KnownArgs::ttf_file),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } },
        args.macro_line_executor,
        args.button_states,
        args.ui_focus.selection_ids.at(id),
        [mle=args.macro_line_executor, on_change=args.arguments.try_at<nlohmann::json>(KnownArgs::on_change)]() {
            if (on_change.has_value() ) {
                mle(*on_change, nullptr, nullptr);
            }
        });
    render_logics.append(
        { parameter_setter_logic, CURRENT_SOURCE_LOCATION },
        1 /* z_order */,
        CURRENT_SOURCE_LOCATION);
}
