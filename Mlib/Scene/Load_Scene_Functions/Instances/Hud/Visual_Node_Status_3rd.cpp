#include "Visual_Node_Status_3rd.hpp"
#include <Mlib/Components/Status_Writer.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Misc/FPath.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_3rd_Logger.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <stdexcept>

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

VisualNodeStatus3rd::VisualNodeStatus3rd(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void VisualNodeStatus3rd::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    auto lo = get_status_writer(node.get(), CURRENT_SOURCE_LOCATION);
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
        args.arguments.at<EFixedArray<float, 2>>(KnownArgs::offset),
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "visual_node_status_3rd",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                VisualNodeStatus3rd{args.renderable_scene()}.execute(args);
            });
    }
} obj;

}
