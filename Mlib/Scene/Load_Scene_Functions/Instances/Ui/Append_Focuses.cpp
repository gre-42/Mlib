#include "Append_Focuses.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(content);
}

AppendFocuses::AppendFocuses(RenderableScene& renderable_scene)
    : LoadRenderableSceneInstanceFunction(renderable_scene)
{}

void AppendFocuses::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    std::scoped_lock lock{ui_focus.focuses.mutex};
    for (Focus focus : args.arguments.at_vector<std::string>(KnownArgs::content, focus_from_string)) {
        ui_focus.focuses.push_back(focus);
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "append_focuses",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                AppendFocuses(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
