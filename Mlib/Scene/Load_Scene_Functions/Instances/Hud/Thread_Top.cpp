#include "Thread_Top.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Thread_Top_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(update_interval_ms);
}

const std::string ThreadTop::key = "thread_top";

LoadSceneJsonUserFunction ThreadTop::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ThreadTop(args.renderable_scene()).execute(args);
};

ThreadTop::ThreadTop(RenderableScene& renderable_scene)
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void ThreadTop::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& loading_logic = object_pool.create<ThreadTopLogic>(
        CURRENT_SOURCE_LOCATION,
        args.arguments.path(KnownArgs::ttf_file),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.arguments.at<UFixedArray<float, 2>>(KnownArgs::position),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        std::chrono::milliseconds{ args.arguments.at<uint32_t>(KnownArgs::update_interval_ms) },
        focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)));
    render_logics.append(
        { loading_logic, CURRENT_SOURCE_LOCATION },
        1,                          // z_order
        CURRENT_SOURCE_LOCATION);
}