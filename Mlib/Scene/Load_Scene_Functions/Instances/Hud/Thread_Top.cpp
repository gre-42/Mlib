#include "Thread_Top.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Thread_Top_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(charset);
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
    ThreadTop(args.physics_scene()).execute(args);
};

ThreadTop::ThreadTop(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ThreadTop::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& loading_logic = object_pool.create<ThreadTopLogic>(
        CURRENT_SOURCE_LOCATION,
        std::make_unique<ExpressionWatcher>(args.macro_line_executor),        
        args.arguments.at<std::string>(KnownArgs::charset),
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
