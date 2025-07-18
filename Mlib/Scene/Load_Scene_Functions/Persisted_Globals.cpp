#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "persisted_globals",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                JsonMacroArguments a;
                for (const auto& [k, v] : args.arguments.items()) {
                    if (v.type() != nlohmann::detail::value_t::string) {
                        THROW_OR_ABORT("Value of \"" + k + "\" is not of type string");
                    }
                    a.set(k, args.ui_focuses[0].get_persisted_selection_id(v.get<std::string>()));
                }
                args.external_json_macro_arguments.merge_and_notify(a);
            });
    }
} obj;

}
