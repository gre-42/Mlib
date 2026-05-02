#include "Create_Tank_Controller.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Tank_Controller.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(left_tire_ids);
DECLARE_ARGUMENT(right_tire_ids);
DECLARE_ARGUMENT(delta_power);
}

CreateTankController::CreateTankController(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateTankController::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    auto rb = get_rigid_body_vehicle(node.get(), CURRENT_SOURCE_LOCATION);
    if (rb->vehicle_controller_ != nullptr) {
        throw std::runtime_error("Tank controller already set");
    }
    std::vector<size_t> left_tire_ids = args.arguments.at<std::vector<size_t>>(KnownArgs::left_tire_ids);
    std::vector<size_t> right_tire_ids = args.arguments.at<std::vector<size_t>>(KnownArgs::right_tire_ids);
    rb->vehicle_controller_ = std::make_unique<TankController>(
        rb,
        left_tire_ids,
        right_tire_ids,
        args.arguments.at<float>(KnownArgs::delta_power) * hp);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_tank_controller",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateTankController{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
