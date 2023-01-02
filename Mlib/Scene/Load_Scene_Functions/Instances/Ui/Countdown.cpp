#include "Countdown.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Render_Logics/Countdown_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(Z_ORDER);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(NSECONDS);
DECLARE_OPTION(PENDING_FOCUS);
DECLARE_OPTION(COUNTING_FOCUS);
DECLARE_OPTION(TEXT);

LoadSceneUserFunction Countdown::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*countdown"
        "\\s+node=([\\w+-.]+)"
        "\\s+z_order=(\\d+)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+nseconds=([\\w+-.]+)"
        "\\s+pending_focus=([\\w+-.]+)"
        "\\s+counting_focus=([\\w+-.]+)"
        "\\s+text=(.*)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        Countdown(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

Countdown::Countdown(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void Countdown::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextGuard rcg{ RenderingContext {
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // ready by CountDownLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // ready by CountDownLogic
        .z_order = safe_stoi(match[Z_ORDER].str())} };                           // read by render_logics
    auto countdown_logic = std::make_shared<CountDownLogic>(
        physics_engine.advance_times_,
        args.fpath(match[TTF_FILE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        safe_stof(match[FONT_HEIGHT].str()),
        safe_stof(match[LINE_DISTANCE].str()),
        safe_stof(match[NSECONDS].str()) * s,
        focus_from_string(match[PENDING_FOCUS].str()),
        focus_from_string(match[COUNTING_FOCUS].str()),
        match[TEXT].str(),
        args.ui_focus.focuses);
    auto node = std::make_unique<SceneNode>();
    physics_engine.advance_times_.add_advance_time(countdown_logic);
    node->destruction_observers.add(*countdown_logic);
    render_logics.append(node.get(), countdown_logic);
    scene.add_root_node(match[NODE].str(), std::move(node));
}
