#include "Update_Requires_Reload.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(indicator);
}

UpdateRequiresReload::UpdateRequiresReload(RenderableScene& renderable_scene)
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void UpdateRequiresReload::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.external_json_macro_arguments.set_and_notify(
        args.arguments.at<std::string>(KnownArgs::indicator),
        !ui_focus.requires_reload().empty());
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "update_requires_reload",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                UpdateRequiresReload{args.renderable_scene()}.execute(args);
            });
    }
} obj;

}
