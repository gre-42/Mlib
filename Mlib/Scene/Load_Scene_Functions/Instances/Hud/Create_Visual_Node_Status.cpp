#include "Create_Visual_Node_Status.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Status_Writer.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Circular_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Text_Logger.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(child);
DECLARE_ARGUMENT(format);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(pointer);
DECLARE_ARGUMENT(tick_radius);
DECLARE_ARGUMENT(pointer_width);
DECLARE_ARGUMENT(pointer_length);
DECLARE_ARGUMENT(minimum_value);
DECLARE_ARGUMENT(maximum_value);
DECLARE_ARGUMENT(blank_angle);
DECLARE_ARGUMENT(ticks);
}

const std::string CreateVisualNodeStatus::key = "visual_node_status";

LoadSceneJsonUserFunction CreateVisualNodeStatus::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateVisualNodeStatus(args.renderable_scene()).execute(args);
};

CreateVisualNodeStatus::CreateVisualNodeStatus(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateVisualNodeStatus::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto lo = &get_status_writer(node);
    if (args.arguments.contains(KnownArgs::child)) {
        lo = &lo->child_status_writer(args.arguments.at<std::vector<std::string>>(KnownArgs::child));
    }
    StatusComponents log_components = status_components_from_string(args.arguments.at<std::string>(KnownArgs::format));
    auto logger = std::make_shared<VisualMovableLogger>(physics_engine.advance_times_);
    auto widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top)));
    if (args.arguments.contains(KnownArgs::ticks)) {
        logger->add_logger(std::make_unique<VisualMovableCircularLogger>(
            *lo,
            log_components,
            args.arguments.path(KnownArgs::ttf_file),
            ColormapWithModifiers{
                .filename = args.arguments.path(KnownArgs::pointer),
                .color_mode = ColorMode::RGBA,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS
            },
            std::move(widget),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::tick_radius)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pointer_width)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pointer_length)),
            args.arguments.at<float>(KnownArgs::minimum_value),
            args.arguments.at<float>(KnownArgs::maximum_value),
            args.arguments.at<float>(KnownArgs::blank_angle) * degrees,
            args.arguments.at_vector<std::string>(KnownArgs::ticks, DisplayTick::from_string)));
    } else {
        logger->add_logger(std::make_unique<VisualMovableTextLogger>(
            *lo,
            log_components,
            args.arguments.path(KnownArgs::ttf_file),
            std::move(widget),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance))));
    }
    physics_engine.advance_times_.add_advance_time(*logger);
    node->clearing_observers.add({ *logger, CURRENT_SOURCE_LOCATION });
    render_logics.append(node.ptr(), logger, 0 /* z_order */);
}
