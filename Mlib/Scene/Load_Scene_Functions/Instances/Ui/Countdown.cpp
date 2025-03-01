#include "Countdown.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Countdown_Logic.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(z_order);
DECLARE_ARGUMENT(charset);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(nseconds);
DECLARE_ARGUMENT(pending_focus);
DECLARE_ARGUMENT(counting_focus);
DECLARE_ARGUMENT(text);
}

const std::string Countdown::key = "countdown";

LoadSceneJsonUserFunction Countdown::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    Countdown(args.renderable_scene()).execute(args);
};

Countdown::Countdown(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void Countdown::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto node = make_unique_scene_node();
    auto& countdown_logic = global_object_pool.create<CountDownLogic>(
        CURRENT_SOURCE_LOCATION,
        node.ref(CURRENT_SOURCE_LOCATION),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::charset, ascii),
        args.arguments.path(KnownArgs::ttf_file),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.arguments.at<UFixedArray<float, 2>>(KnownArgs::position),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        args.arguments.at<float>(KnownArgs::nseconds) * seconds,
        focus_from_string(args.arguments.at<std::string>(KnownArgs::pending_focus)),
        focus_from_string(args.arguments.at<std::string>(KnownArgs::counting_focus)),
        args.arguments.at<std::string>(KnownArgs::text),
        args.ui_focus.focuses);
    countdown_logic.on_node_clear.add([&countdown_logic]() { global_object_pool.remove(countdown_logic); }, CURRENT_SOURCE_LOCATION);
    physics_engine.advance_times_.add_advance_time({ countdown_logic, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    render_logics.append(
        { countdown_logic, CURRENT_SOURCE_LOCATION },
        args.arguments.at<int>(KnownArgs::z_order),
        CURRENT_SOURCE_LOCATION);
    scene.add_root_node(
        args.arguments.at<std::string>(KnownArgs::node),
        std::move(node),
        RenderingDynamics::MOVING,
        RenderingStrategies::OBJECT);
}
