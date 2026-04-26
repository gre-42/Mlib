#include "Set_Capsule_Surface_Normal.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Collision/Detect/Normal_On_Capsule.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(position);
}

SetCapsuleSurfaceNormal::SetCapsuleSurfaceNormal(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetCapsuleSurfaceNormal::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    auto rb = get_rigid_body_vehicle(node.get(), CURRENT_SOURCE_LOCATION);
    auto R = args.arguments.at<EFixedArray<float, 3, 3>>(KnownArgs::rotation);
    rb->set_surface_normal(std::make_unique<NormalOnCapsule>(
        rb,
        TransformationMatrix<float, ScenePos, 3>{
            R,
            args.arguments.at<EFixedArray<ScenePos, 3>>(KnownArgs::position) * (ScenePos)meters}));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_capsule_surface_normal",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetCapsuleSurfaceNormal{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
