#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Resource_Managers/Texture_Warn_Flags.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(flags);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "suppress_texture_warnings",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                RenderingContextStack::primary_rendering_resources().set_suppressed_warnings(
                    args.arguments.try_path_or_variable(KnownArgs::filename),
                    texture_warn_flags_from_string(args.arguments.at<std::string>(KnownArgs::flags)));
            });
    }
} obj;

}
