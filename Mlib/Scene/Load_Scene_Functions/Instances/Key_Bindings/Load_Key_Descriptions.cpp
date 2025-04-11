#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Descriptions.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filename);
}

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "load_key_descriptions",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto lock = args.key_descriptions.lock_exclusive_for(
                    std::chrono::seconds(2),
                    "Key descriptions");
                auto& cfg = *lock;
                if (cfg.has_value()) {
                    THROW_OR_ABORT("Key descriptions already initialized");
                }
                cfg.emplace().load(args.arguments.path(KnownArgs::filename));
            });
    }
} obj;

}
