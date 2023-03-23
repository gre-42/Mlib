#include "Create_Visual_Node_Status.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Circular_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Text_Logger.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(CHILD);
DECLARE_OPTION(FORMAT);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(POINTER);
DECLARE_OPTION(TICK_RADIUS);
DECLARE_OPTION(POINTER_WIDTH);
DECLARE_OPTION(POINTER_LENGTH);
DECLARE_OPTION(MINIMUM_VALUE);
DECLARE_OPTION(MAXIMUM_VALUE);
DECLARE_OPTION(BLANK_ANGLE);
DECLARE_OPTION(TICKS);

LoadSceneUserFunction CreateVisualNodeStatus::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*visual_node_status"
        "\\s+node=([\\w+-.]+)"
        "(?:,\\s+child=([^,]+))?"
        ",\\s+format=([\\w|]+)"
        ",\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        ",\\s+left=(\\w+)"
        ",\\s+right=(\\w+)"
        ",\\s+bottom=(\\w+)"
        ",\\s+top=(\\w+)"
        ",\\s+font_height=(\\w+)"
        ",\\s+line_distance=(\\w+)"
        "(?:,\\s+pointer=([\\w+-. \\(\\)/]+)"
        ",\\s+tick_radius=([\\w+-.]+)"
        ",\\s+pointer_width=([\\w+-.]+)"
        ",\\s+pointer_length=([\\w+-.]+)"
        ",\\s+minimum_value=([\\w+-.]+)"
        ",\\s+maximum_value=([\\w+-.]+)"
        ",\\s+blank_angle=([\\w+-.]+)"
        ",\\s+ticks=(.*))?$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateVisualNodeStatus(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateVisualNodeStatus::CreateVisualNodeStatus(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateVisualNodeStatus::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto lo = dynamic_cast<StatusWriter*>(&node.get_absolute_movable());
    if (lo == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a status writer");
    }
    if (match[CHILD].matched) {
        lo = &lo->child_status_writer(string_to_vector(match[CHILD].str()));
    }
    StatusComponents log_components = status_components_from_string(match[FORMAT].str());
    auto logger = std::make_shared<VisualMovableLogger>(physics_engine.advance_times_);
    auto widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(match[LEFT].str()),
        args.layout_constraints.get_pixels(match[RIGHT].str()),
        args.layout_constraints.get_pixels(match[BOTTOM].str()),
        args.layout_constraints.get_pixels(match[TOP].str()));
    if (match[TICKS].matched) {
        logger->add_logger(std::make_unique<VisualMovableCircularLogger>(
            *lo,
            log_components,
            args.fpath(match[TTF_FILE].str()).path,
            args.fpath(match[POINTER].str()).path,
            std::move(widget),
            args.layout_constraints.get_pixels(match[FONT_HEIGHT].str()),
            args.layout_constraints.get_pixels(match[TICK_RADIUS].str()),
            args.layout_constraints.get_pixels(match[POINTER_WIDTH].str()),
            args.layout_constraints.get_pixels(match[POINTER_LENGTH].str()),
            safe_stof(match[MINIMUM_VALUE].str()),
            safe_stof(match[MAXIMUM_VALUE].str()),
            safe_stof(match[BLANK_ANGLE].str()) * degrees,
            string_to_vector(match[TICKS].str(), DisplayTick::from_string)));
    } else {
        logger->add_logger(std::make_unique<VisualMovableTextLogger>(
            *lo,
            log_components,
            args.fpath(match[TTF_FILE].str()).path,
            std::move(widget),
            args.layout_constraints.get_pixels(match[FONT_HEIGHT].str()),
            args.layout_constraints.get_pixels(match[LINE_DISTANCE].str())));
    }
    physics_engine.advance_times_.add_advance_time(*logger);
    node.destruction_observers.add(*logger);
    render_logics.append(&node, logger);
}
