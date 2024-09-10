#include "Create_Delta_Engine.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Delta_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
    CreateDeltaEngine(args.renderable_scene()).execute(args);
};

CreateDeltaEngine::CreateDeltaEngine(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDeltaEngine::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::rigid_body), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    auto ep = rb.delta_engines_.try_emplace(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name));
    if (!ep.second) {
        THROW_OR_ABORT("Delta engine with name \"" + args.arguments.at<std::string>(KnownArgs::name) + "\" already exists");
    }
}
