#include "Fit_Canvas_To_Renderables.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Modifiers/Fit_Canvas_To_Renderables.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(render_pass);
}

const std::string FitCanvasToRenderables::key = "fit_canvas_to_renderables";

LoadSceneJsonUserFunction FitCanvasToRenderables::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    FitCanvasToRenderables(args.physics_scene()).execute(args);
};

FitCanvasToRenderables::FitCanvasToRenderables(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void FitCanvasToRenderables::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto camera = node->get_camera(DP_LOC);
    auto* ortho_camera = dynamic_cast<OrthoCamera*>(&camera.get());
    if (ortho_camera == nullptr) {
        THROW_OR_ABORT("Camera is not an ortho-camera");
    }
    fit_canvas_to_renderables(
        scene,
        node->absolute_view_matrix(),
        *ortho_camera,
        external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::render_pass)));
}
