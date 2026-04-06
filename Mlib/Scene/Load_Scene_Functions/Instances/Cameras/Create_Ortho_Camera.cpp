#include "Create_Ortho_Camera.hpp"
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera_Config_Json.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(config);
DECLARE_ARGUMENT(requires_postprocessing);
}

const std::string CreateOrthoCamera::key = "ortho_camera";

LoadSceneJsonUserFunction CreateOrthoCamera::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    CreateOrthoCamera(args.physics_scene()).execute(args);
};

CreateOrthoCamera::CreateOrthoCamera(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateOrthoCamera::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    auto oc = std::make_unique<OrthoCamera>(
        args.arguments.at<OrthoCameraConfig>(KnownArgs::config),
        OrthoCamera::Postprocessing::ENABLED);
    oc->set_requires_postprocessing(args.arguments.at<bool>(KnownArgs::requires_postprocessing));
    node->set_camera(std::move(oc));
}
