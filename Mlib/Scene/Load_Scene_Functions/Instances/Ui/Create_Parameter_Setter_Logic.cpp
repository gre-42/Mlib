#include "Create_Parameter_Setter_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Verbose_Vector.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Parameter_Setter_Logic.hpp>
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
DECLARE_ARGUMENT(charset);
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
DECLARE_ARGUMENT(on_execute);
DECLARE_ARGUMENT(assets);
DECLARE_ARGUMENT(database_filter);
DECLARE_ARGUMENT(hide_if_trivial);
DECLARE_ARGUMENT(parameters);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
DECLARE_ARGUMENT(persistent);
DECLARE_ARGUMENT(local_user_id);
}

CreateParameterSetterLogic::CreateParameterSetterLogic(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateParameterSetterLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto id = args.arguments.at<std::string>(KnownArgs::id);
    std::list<ReplacementParameter> rps;
    if (args.arguments.contains(KnownArgs::parameters)) {
        rps = args.arguments.at<std::list<ReplacementParameter>>(KnownArgs::parameters);
    }
    if (args.arguments.contains(KnownArgs::assets)) {
        const auto& assets = args.asset_references[args.arguments.at<std::string>(KnownArgs::assets)];
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
                for (auto&& v : a.rp.database.at<std::vector<std::string>>(f.variable)) {
                    ents.insert(std::move(v));
                }
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Error processing asset \"" + a.rp.id + "\": " + e.what());
            }
        }
        rps.remove_if([&ents](const ReplacementParameter& rp) { return (!ents.contains(rp.id)); });
    }
    rps.remove_if([&](const ReplacementParameter& rp){
        return !args.macro_line_executor.eval(rp.required.fixed);
    });
    if (args.arguments.at<bool>(KnownArgs::hide_if_trivial, false)) {
        if (rps.empty()) {
            return;
        }
        if (rps.size() == 1) {
            const auto& rp = rps.front();
            if (!rp.on_before_select.is_null()) {
                args.macro_line_executor(rp.on_before_select, nullptr);
            }
            return;
        }
    }
    auto focus_filter = FocusFilter{
        .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
        .submenu_ids = args.arguments.at<std::set<std::string>>(KnownArgs::submenus, { id }) };
    BooleanExpression required;
    if (auto r = args.arguments.try_at(KnownArgs::required); r.has_value()) {
        expression_from_json(*r, required);
    }
    auto& header = ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title = args.arguments.at<std::string>(KnownArgs::title),
            .icon = args.arguments.at_non_null<std::string>(KnownArgs::icon, "")
        },
        focus_filter,
        args.arguments.at<size_t>(KnownArgs::deflt));
    std::function<void()> on_execute;
    if (auto ooe = args.arguments.try_at(KnownArgs::on_execute); ooe.has_value()) {
        on_execute = [mle=args.macro_line_executor, oe=*ooe]() {
            mle(oe, nullptr);
        };
    }
    auto local_user_id = args.arguments.at<uint32_t>(KnownArgs::local_user_id);
    auto& parameter_setter_logic = object_pool.create<ParameterSetterLogic>(
        CURRENT_SOURCE_LOCATION,
        std::move(required),
        std::move(id),
        std::vector<ReplacementParameter>{rps.begin(), rps.end()},
        args.confirm_button_press.get(local_user_id),
        args.arguments.at<std::string>(KnownArgs::charset),
        args.arguments.path(KnownArgs::ttf_file),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        focus_filter,
        std::make_unique<ExpressionWatcher>(args.macro_line_executor),
        ui_focus,
        header,
        args.arguments.at<std::string>(KnownArgs::persistent, ""),
        args.button_states,
        local_user_id,
        [mle=args.macro_line_executor, on_change=args.arguments.try_at(KnownArgs::on_change)]() {
            if (on_change.has_value() ) {
                mle(*on_change, nullptr);
            }
        },
        on_execute);
    render_logics.append(
        { parameter_setter_logic, CURRENT_SOURCE_LOCATION },
        1 /* z_order */,
        CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "parameter_setter",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateParameterSetterLogic(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
