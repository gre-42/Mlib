#include "Create_Delta_Engine.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Delta_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(rigid_body);
DECLARE_ARGUMENT(name);
}

const std::string CreateDeltaEngine::key = "create_delta_engine";

LoadSceneJsonUserFunction CreateDeltaEngine::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateDeltaEngine(args.physics_scene()).execute(args);
};

CreateDeltaEngine::CreateDeltaEngine(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateDeltaEngine::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::rigid_body), CURRENT_SOURCE_LOCATION);
    auto rb = get_rigid_body_vehicle(node.get(), CURRENT_SOURCE_LOCATION);
    auto ep = rb->delta_engines_.try_emplace(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name));
    if (!ep.second) {
        throw std::runtime_error("Delta engine with name \"" + args.arguments.at<std::string>(KnownArgs::name) + "\" already exists");
    }
}
