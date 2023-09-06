#include "Fit_Canvas_To_Renderables.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Modifiers/Fit_Canvas_To_Renderables.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(render_pass);
}

const std::string FitCanvasToRenderables::key = "fit_canvas_to_renderables";

LoadSceneJsonUserFunction FitCanvasToRenderables::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    FitCanvasToRenderables(args.renderable_scene()).execute(args);
};

FitCanvasToRenderables::FitCanvasToRenderables(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void FitCanvasToRenderables::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto* camera = dynamic_cast<OrthoCamera*>(&node->get_camera());
    if (camera == nullptr) {
        THROW_OR_ABORT("Camera is not an ortho-camera");
    }
    fit_canvas_to_renderables(
        scene,
        node->absolute_view_matrix(),
        *camera,
        external_render_pass_type_from_string(args.arguments.at<std::string>(KnownArgs::render_pass)));
}
