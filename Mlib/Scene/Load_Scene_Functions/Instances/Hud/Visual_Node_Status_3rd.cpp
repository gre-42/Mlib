#include "Visual_Node_Status_3rd.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_3rd_Logger.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(FORMAT);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(OFFSET_X);
DECLARE_OPTION(OFFSET_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);

LoadSceneUserFunction VisualNodeStatus3rd::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*visual_node_status_3rd"
        "\\s+node=([\\w+-.]+)"
        "\\s+format=(\\d+)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+offset=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=(\\w+)"
        "\\s+line_distance=(\\w+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        VisualNodeStatus3rd(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

VisualNodeStatus3rd::VisualNodeStatus3rd(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void VisualNodeStatus3rd::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto lo = dynamic_cast<StatusWriter*>(&node.get_absolute_movable());
    if (lo == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a status writer");
    }
    StatusComponents log_components = (StatusComponents)safe_stoi(match[FORMAT].str());
    auto logger = std::make_shared<VisualMovable3rdLogger>(
        scene_logic,
        node,
        physics_engine.advance_times_,
        lo,
        log_components,
        args.fpath(match[TTF_FILE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[OFFSET_X].str()),
            safe_stof(match[OFFSET_Y].str())},
        args.layout_constraints.get_scalar(match[FONT_HEIGHT].str()),
        args.layout_constraints.get_scalar(match[LINE_DISTANCE].str()));
    render_logics.append(&node, logger);
    physics_engine.advance_times_.add_advance_time(logger);
}
