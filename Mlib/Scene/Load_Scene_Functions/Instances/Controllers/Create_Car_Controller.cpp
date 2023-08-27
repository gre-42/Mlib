#include "Create_Car_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Car_Controller.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(front_engine);
DECLARE_ARGUMENT(rear_engine);
DECLARE_ARGUMENT(front_tire_ids);
DECLARE_ARGUMENT(asset_id);
}

const std::string CreateCarController::key = "create_car_controller";

LoadSceneJsonUserFunction CreateCarController::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateCarController(args.renderable_scene()).execute(args);
};

CreateCarController::CreateCarController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCarController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node->get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Car movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Car controller already set");
    }
    auto asset_id = args.arguments.at<std::string>(KnownArgs::asset_id);
    const auto& vars = args
        .asset_references
        .get("vehicles")
        .at(asset_id)
        .rp;
    auto front_tire_ids = args.arguments.at_non_null<std::vector<size_t>>(KnownArgs::front_tire_ids, {});
    rb->vehicle_controller_ = std::make_unique<CarController>(
        *rb,
        args.arguments.at<std::string>(KnownArgs::front_engine),
        args.arguments.at<std::string>(KnownArgs::rear_engine),
        front_tire_ids,
        front_tire_ids.empty()
            ? NAN
            : vars.database.at<float>("MAX_TIRE_ANGLE") * degrees,
        physics_engine);
}
