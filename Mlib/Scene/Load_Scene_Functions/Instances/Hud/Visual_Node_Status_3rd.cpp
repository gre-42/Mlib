#include "Visual_Node_Status_3rd.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Status_Writer.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_3rd_Logger.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(format);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(offset);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
}

const std::string VisualNodeStatus3rd::key = "visual_node_status_3rd";

LoadSceneJsonUserFunction VisualNodeStatus3rd::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    VisualNodeStatus3rd(args.renderable_scene()).execute(args);
};

VisualNodeStatus3rd::VisualNodeStatus3rd(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void VisualNodeStatus3rd::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& lo = get_status_writer(node);
    StatusComponents log_components = status_components_from_string(args.arguments.at<std::string>(KnownArgs::format));
    global_object_pool.create<VisualMovable3rdLogger>(
        CURRENT_SOURCE_LOCATION,
        scene_logic,
        node,
        render_logics,
        physics_engine.advance_times_,
        lo,
        log_components,
        std::make_unique<ExpressionWatcher>(args.macro_line_executor),
        args.arguments.at<std::string>(KnownArgs::charset),
        args.arguments.path(KnownArgs::ttf_file),
        args.arguments.at<UFixedArray<float, 2>>(KnownArgs::offset),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)));
}
