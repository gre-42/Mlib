#include "Visual_Node_Status_3rd.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_3rd_Logger.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>

using namespace Mlib;

LoadSceneUserFunction VisualNodeStatus3rd::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*visual_node_status_3rd"
        "\\s+node=([\\w+-.]+)"
        "\\s+format=(\\d+)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+offset=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)$");
    std::smatch match;
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
    auto& node = scene.get_node(match[1].str());
    auto lo = dynamic_cast<StatusWriter*>(&node.get_absolute_movable());
    if (lo == nullptr) {
        throw std::runtime_error("Absolute movable is not a status writer");
    }
    StatusComponents log_components = (StatusComponents)safe_stoi(match[2].str());
    auto logger = std::make_shared<VisualMovable3rdLogger>(
        scene_logic,
        node,
        physics_engine.advance_times_,
        lo,
        log_components,
        args.fpath(match[3].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[4].str()),
            safe_stof(match[5].str())},
        safe_stof(match[6].str()),
        safe_stof(match[7].str()));
    render_logics.append(&node, logger);
    physics_engine.advance_times_.add_advance_time(logger);
}
