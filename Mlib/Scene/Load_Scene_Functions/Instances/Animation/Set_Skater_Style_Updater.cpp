#include "Set_Skater_Style_Updater.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Animation/Skater_Animation_Updater.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(skater_node);
DECLARE_ARGUMENT(skateboard_node);
DECLARE_ARGUMENT(resource);
}

SetSkaterStyleUpdater::SetSkaterStyleUpdater(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetSkaterStyleUpdater::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DanglingRef<SceneNode> skater_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::skater_node), DP_LOC);
    DanglingRef<SceneNode> skateboard_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::skateboard_node), DP_LOC);
    std::string resource = args.arguments.at<std::string>(KnownArgs::resource);
    auto& rb = get_rigid_body_vehicle(skater_node);
    if (rb.animation_state_updater_ != nullptr) {
        THROW_OR_ABORT("Rigid body already has a style updater");
    }
    auto updater = std::make_unique<SkaterAnimationUpdater>(rb, skateboard_node, resource);
    AnimationStateUpdater* ptr = updater.get();
    skater_node->set_animation_state_updater(std::move(updater));
    rb.animation_state_updater_ = { ptr, CURRENT_SOURCE_LOCATION };
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_skater_style_updater",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetSkaterStyleUpdater(args.physics_scene()).execute(args);
            });
    }
} obj;

}
