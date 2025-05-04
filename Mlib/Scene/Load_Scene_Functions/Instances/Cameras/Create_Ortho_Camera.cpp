#include "Create_Ortho_Camera.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(near_plane);
DECLARE_ARGUMENT(far_plane);
DECLARE_ARGUMENT(left_plane);
DECLARE_ARGUMENT(right_plane);
DECLARE_ARGUMENT(bottom_plane);
DECLARE_ARGUMENT(top_plane);
DECLARE_ARGUMENT(requires_postprocessing);
}

const std::string CreateOrthoCamera::key = "ortho_camera";

LoadSceneJsonUserFunction CreateOrthoCamera::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    CreateOrthoCamera(args.renderable_scene()).execute(args);
};

CreateOrthoCamera::CreateOrthoCamera(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateOrthoCamera::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto oc = std::make_unique<OrthoCamera>(
        OrthoCameraConfig(),
        OrthoCamera::Postprocessing::ENABLED);
    oc->set_near_plane(args.arguments.at<float>(KnownArgs::near_plane));
    oc->set_far_plane(args.arguments.at<float>(KnownArgs::far_plane));
    oc->set_left_plane(args.arguments.at<float>(KnownArgs::left_plane));
    oc->set_right_plane(args.arguments.at<float>(KnownArgs::right_plane));
    oc->set_bottom_plane(args.arguments.at<float>(KnownArgs::bottom_plane));
    oc->set_top_plane(args.arguments.at<float>(KnownArgs::top_plane));
    oc->set_requires_postprocessing(args.arguments.at<bool>(KnownArgs::requires_postprocessing));
    node->set_camera(std::move(oc));
}
