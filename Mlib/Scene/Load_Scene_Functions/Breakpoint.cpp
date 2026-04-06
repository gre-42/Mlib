#include <Mlib/Json/Json_Object_File.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(path);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "breakpoint",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                JsonObjectFile j;
                j.load_from_file(args.arguments.path_or_variable(KnownArgs::path).local_path());
                auto name = args.arguments.at<std::string>(KnownArgs::name);
                if (j.at<bool>(name)) {
                    lerr() << "Triggering breakpoint \"" << name << '"';
                    throw std::runtime_error("Breakpoint \"" + name + '"');
                }
            });
    }
} obj;

}
