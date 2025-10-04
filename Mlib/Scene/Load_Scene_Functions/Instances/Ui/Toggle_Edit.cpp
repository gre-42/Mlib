#include "Toggle_Edit.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(menu_id);
DECLARE_ARGUMENT(entry_id);
DECLARE_ARGUMENT(persisted);
DECLARE_ARGUMENT(global);
DECLARE_ARGUMENT(value);
}

ToggleEdit::ToggleEdit(RenderableScene& renderable_scene)
    : LoadRenderableSceneInstanceFunction(renderable_scene)
{}

void ToggleEdit::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    std::scoped_lock lock{ ui_focus.edit_mutex };
    if (ui_focus.editing.has_value()) {
        if (!ui_focus.editing->global.empty()) {
            args.external_json_macro_arguments.set_and_notify(
                ui_focus.editing->global,
                ui_focus.editing->value);
        }
        if (!ui_focus.editing->persisted.empty()) {
            ui_focus.set_persisted_selection_id(ui_focus.editing->persisted, ui_focus.editing->value, PersistedValueType::CUSTOM);
        }
        ui_focus.editing.reset();
    } else {
        ui_focus.editing = EditFocus{
            .menu_id = args.arguments.at<std::string>(KnownArgs::menu_id),
            .entry_id = args.arguments.at<std::string>(KnownArgs::entry_id),
            .persisted = args.arguments.at<std::string>(KnownArgs::persisted),
            .global = args.arguments.at<std::string>(KnownArgs::global),
            .value = args.arguments.at<std::string>(KnownArgs::value)};
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "toggle_edit",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ToggleEdit(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
