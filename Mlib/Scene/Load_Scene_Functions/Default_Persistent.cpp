#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(user_id);
DECLARE_ARGUMENT(variables);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "default_persistent",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                auto user_id = args.arguments.at<uint32_t>(KnownArgs::user_id);
                auto variables = args.arguments.at(KnownArgs::variables);
                for (const auto& [k, v] : variables.items()) {
                    if (v.type() != nlohmann::detail::value_t::string) {
                        THROW_OR_ABORT("Value of \"" + k + "\" is not of type string");
                    }
                    args.ui_focuses[user_id].set_persisted_selection_id(k, v.get<std::string>(), PersistedValueType::DEFAULT);
                }
            });
    }
} obj;

}
