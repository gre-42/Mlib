#include "Create_Perspective_Camera.hpp"
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(y_fov);
DECLARE_ARGUMENT(near_plane);
DECLARE_ARGUMENT(far_plane);
DECLARE_ARGUMENT(requires_postprocessing);
}

CreatePerspectiveCamera::CreatePerspectiveCamera(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreatePerspectiveCamera::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);

    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    auto pc = std::make_unique<PerspectiveCamera>(
        PerspectiveCameraConfig(),
        PerspectiveCamera::Postprocessing::ENABLED);
    pc->set_y_fov(args.arguments.at<float>(KnownArgs::y_fov) * degrees);
    pc->set_near_plane(args.arguments.at<float>(KnownArgs::near_plane));
    pc->set_far_plane(args.arguments.at<float>(KnownArgs::far_plane));
    pc->set_requires_postprocessing(args.arguments.at<bool>(KnownArgs::requires_postprocessing));
    node->set_camera(std::move(pc));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "perspective_camera",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreatePerspectiveCamera{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
