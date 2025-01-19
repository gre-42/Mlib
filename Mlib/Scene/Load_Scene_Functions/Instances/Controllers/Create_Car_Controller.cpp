#include "Create_Car_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
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
DECLARE_ARGUMENT(max_tire_angle);
DECLARE_ARGUMENT(tire_angle_velocities);
DECLARE_ARGUMENT(tire_angles);
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

inline float stov(float v) {
    return v * kph;
}

inline float stoa(float v) {
    return v * degrees;
}

void CreateCarController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    if (rb.vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Car controller already set");
    }
    auto front_tire_ids = args.arguments.at_non_null<std::vector<size_t>>(KnownArgs::front_tire_ids, {});
    rb.vehicle_controller_ = std::make_unique<CarController>(
        rb,
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::front_engine),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::rear_engine),
        front_tire_ids,
        front_tire_ids.empty()
            ? NAN
            : stoa(args.arguments.at<float>(KnownArgs::max_tire_angle)),
        front_tire_ids.empty()
            ? Interp<float>{{}, {}}
            : Interp<float>{
                args.arguments.at_vector<float>(KnownArgs::tire_angle_velocities, stov),
                args.arguments.at_vector<float>(KnownArgs::tire_angles, stoa),
                OutOfRangeBehavior::CLAMP},
        physics_engine);
}
