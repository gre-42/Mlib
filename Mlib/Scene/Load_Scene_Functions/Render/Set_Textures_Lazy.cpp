#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(command);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_textures_lazy",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto command = args.arguments.at(KnownArgs::command);
                // args.render_set_fps.set_fps.execute([mle=args.macro_line_executor, command](){mle(command, nullptr, nullptr);});
                // append_render_allocator([mle=args.macro_line_executor, command](){mle(command, nullptr, nullptr);});
                RenderingContextStack::primary_rendering_resources().set_textures_lazy(
                    [mle=args.macro_line_executor, command](){mle(command, nullptr);});
            });
    }
} obj;

}
