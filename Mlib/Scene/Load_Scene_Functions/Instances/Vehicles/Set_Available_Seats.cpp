#include "Set_Available_Seats.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(value);
}

SetAvailableSeats::SetAvailableSeats(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetAvailableSeats::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    rb.drivers_.set_seats(args.arguments.at<std::vector<std::string>>(KnownArgs::value));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_available_seats",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetAvailableSeats(args.physics_scene()).execute(args);
            });
    }
} obj;

}
