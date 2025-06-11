#include "Set_Textures_Lazy.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(command);
}

const std::string SetTexturesLazy::key = "set_textures_lazy";

LoadSceneJsonUserFunction SetTexturesLazy::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto command = args.arguments.at(KnownArgs::command);
    // args.render_set_fps.set_fps.execute([mle=args.macro_line_executor, command](){mle(command, nullptr, nullptr);});
    // append_render_allocator([mle=args.macro_line_executor, command](){mle(command, nullptr, nullptr);});
    RenderingContextStack::primary_rendering_resources().set_textures_lazy(
        [mle=args.macro_line_executor, command](){mle(command, nullptr);});
};
