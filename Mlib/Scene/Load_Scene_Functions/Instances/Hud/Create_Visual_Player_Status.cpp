#include "Create_Visual_Player_Status.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Status_Writer.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Circular_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Text_Logger.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(child);
DECLARE_ARGUMENT(format);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(circular);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
}

namespace CircularArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(pointer);
DECLARE_ARGUMENT(tick_radius);
DECLARE_ARGUMENT(pointer_width);
DECLARE_ARGUMENT(pointer_length);
DECLARE_ARGUMENT(minimum_value);
DECLARE_ARGUMENT(maximum_value);
DECLARE_ARGUMENT(blank_angle);
DECLARE_ARGUMENT(ticks);
}

const std::string CreateVisualPlayerStatus::key = "visual_player_status";

LoadSceneJsonUserFunction CreateVisualPlayerStatus::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    if (args.arguments.contains(KnownArgs::circular)) {
        args.arguments.child(KnownArgs::circular).validate(CircularArgs::options);
    }
    CreateVisualPlayerStatus(args.physics_scene()).execute(args);
};

CreateVisualPlayerStatus::CreateVisualPlayerStatus(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateVisualPlayerStatus::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    DanglingRef<SceneNode> node = player->scene_node();
    auto lo = &get_status_writer(node);
    if (args.arguments.contains(KnownArgs::child)) {
        lo = &lo->child_status_writer(args.arguments.at<std::vector<VariableAndHash<std::string>>>(KnownArgs::child));
    }
    StatusComponents log_components = status_components_from_string(args.arguments.at<std::string>(KnownArgs::format));
    auto& logger = object_pool.create<VisualMovableLogger>(
        CURRENT_SOURCE_LOCATION,
        object_pool,
        physics_engine.advance_times_,
        render_logics,
        node,
        player.ptr(),
        FocusFilter{
            .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
            .submenu_ids = string_to_set(args.arguments.at<std::string>(KnownArgs::submenus, {}))});
    auto widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top)));
    if (auto c = args.arguments.try_get_child(KnownArgs::circular); c.has_value()) {
        logger.add_logger(std::make_unique<VisualMovableCircularLogger>(
            *lo,
            log_components,
            std::make_unique<ExpressionWatcher>(args.macro_line_executor),        
            args.arguments.at<std::string>(KnownArgs::charset),
            args.arguments.path(KnownArgs::ttf_file),
            ColormapWithModifiers{
                .filename = VariableAndHash{c->path(CircularArgs::pointer)},
                .color_mode = ColorMode::RGBA,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS
            }.compute_hash(),
            std::move(widget),
            args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
            args.layout_constraints.get_pixels(c->at<std::string>(CircularArgs::tick_radius)),
            args.layout_constraints.get_pixels(c->at<std::string>(CircularArgs::pointer_width)),
            args.layout_constraints.get_pixels(c->at<std::string>(CircularArgs::pointer_length)),
            c->at<float>(CircularArgs::minimum_value),
            c->at<float>(CircularArgs::maximum_value),
            c->at<float>(CircularArgs::blank_angle) * degrees,
            c->at_vector<std::string>(CircularArgs::ticks, DisplayTick::from_string)));
    } else {
        logger.add_logger(std::make_unique<VisualMovableTextLogger>(
            *lo,
            log_components,
            std::make_unique<ExpressionWatcher>(args.macro_line_executor),        
            args.arguments.at<std::string>(KnownArgs::charset),
            args.arguments.path(KnownArgs::ttf_file),
            std::move(widget),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance))));
    }
}
