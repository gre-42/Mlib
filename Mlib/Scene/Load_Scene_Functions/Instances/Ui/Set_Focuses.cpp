#include "Set_Focuses.hpp"
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
DECLARE_ARGUMENT(focuses);
}

SetFocuses::SetFocuses(RenderableScene& renderable_scene)
    : LoadRenderableSceneInstanceFunction{renderable_scene}
{}

void SetFocuses::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    std::scoped_lock lock{ui_focus.focuses.mutex};
    ui_focus.focuses.set_focuses(args.arguments.at_vector<std::string>(KnownArgs::focuses, single_focus_from_string));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_focuses",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetFocuses(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
