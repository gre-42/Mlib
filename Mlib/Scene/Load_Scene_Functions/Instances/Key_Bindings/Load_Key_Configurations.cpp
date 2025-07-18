#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(user_id);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(fallback_filename);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "load_key_configurations",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto lock = args.key_configurations.lock_exclusive_for(
                    std::chrono::seconds(2),
                    "Key configurations");
                lock->load(
                    args.arguments.at<uint32_t>(KnownArgs::user_id),
                    args.arguments.path(KnownArgs::filename),
                    args.arguments.path(KnownArgs::fallback_filename));
            });
    }
} obj;

}
