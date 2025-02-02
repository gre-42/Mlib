#include "Set_Render_Fps.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Time/Fps/Realtime_Dependent_Fps.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(fps);
}

const std::string SetRenderFps::key = "set_render_fps";

LoadSceneJsonUserFunction SetRenderFps::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.render_set_fps.set_fps.execute(
        [&rts=args.render_set_fps.rts, fps=args.arguments.at<float>(KnownArgs::fps)]()
        {rts.set_dt(1.f / fps);});
};
