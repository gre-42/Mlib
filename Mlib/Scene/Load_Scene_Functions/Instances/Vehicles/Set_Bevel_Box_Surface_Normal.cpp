#include "Set_Bevel_Box_Surface_Normal.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Collision/Detect/Normal_On_Bevel_Box.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
DECLARE_ARGUMENT(radius);
}

SetBevelBoxSurfaceNormal::SetBevelBoxSurfaceNormal(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetBevelBoxSurfaceNormal::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    auto rb = get_rigid_body_vehicle(node.get(), CURRENT_SOURCE_LOCATION);
    rb->set_surface_normal(std::make_unique<NormalOnBevelBox>(
        rb,
        AxisAlignedBoundingBox<float, 3>::from_min_max(
            args.arguments.at<EFixedArray<float, 3>>(KnownArgs::min) * meters,
            args.arguments.at<EFixedArray<float, 3>>(KnownArgs::max) * meters),
        args.arguments.at<float>(KnownArgs::radius) * meters));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_bevel_box_surface_normal",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetBevelBoxSurfaceNormal{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
