#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "default_persistent",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                for (const auto& [k, v] : args.arguments.items()) {
                    if (v.type() != nlohmann::detail::value_t::string) {
                        THROW_OR_ABORT("Value of \"" + k + "\" is not of type string");
                    }
                    args.ui_focus.set_persisted_selection_id(k, v.get<std::string>(), PersistedValueType::DEFAULT);
                }
            });
    }
} obj;

}
