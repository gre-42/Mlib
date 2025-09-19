#include "Create_Heli_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Heli_Controller.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(tire_ids);
DECLARE_ARGUMENT(tire_angles);
DECLARE_ARGUMENT(main_rotor_id);
DECLARE_ARGUMENT(pitch_multiplier);
DECLARE_ARGUMENT(yaw_multiplier);
DECLARE_ARGUMENT(roll_multiplier);
DECLARE_ARGUMENT(ascend_p);
DECLARE_ARGUMENT(ascend_i);
DECLARE_ARGUMENT(ascend_d);
DECLARE_ARGUMENT(ascend_a);
DECLARE_ARGUMENT(vehicle_domain);
}

const std::string CreateHeliController::key = "create_heli_controller";

static float from_degrees(float v) {
    return v * degrees;
}

LoadSceneJsonUserFunction CreateHeliController::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateHeliController(args.physics_scene()).execute(args);
};

CreateHeliController::CreateHeliController(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateHeliController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    if (rb.vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Heli controller already set");
    }
    std::vector<size_t> tire_ids = args.arguments.at<std::vector<size_t>>(KnownArgs::tire_ids);
    std::vector<float> tire_angles = args.arguments.at_vector<float>(KnownArgs::tire_angles, from_degrees);
    if (tire_ids.size() != tire_angles.size()) {
        THROW_OR_ABORT("Tire IDs and angles have different lengths");
    }
    std::map<size_t, float> tire_angles_map;
    for (size_t i = 0; i < tire_ids.size(); ++i) {
        if (!tire_angles_map.insert({ tire_ids[i], tire_angles[i] }).second) {
            THROW_OR_ABORT("Duplicate tire ID");
        }
    }
    rb.vehicle_controller_ = std::make_unique<HeliController>(
        rb,
        tire_angles_map,
        args.arguments.at<size_t>(KnownArgs::main_rotor_id),
        FixedArray<float, 3>{
            args.arguments.at<float>(KnownArgs::pitch_multiplier),
            args.arguments.at<float>(KnownArgs::yaw_multiplier) * W,
            args.arguments.at<float>(KnownArgs::roll_multiplier)},
        PidController<double, double>{
            args.arguments.at<float>(KnownArgs::ascend_p) * W,
            args.arguments.at<float>(KnownArgs::ascend_i) * W,
            args.arguments.at<float>(KnownArgs::ascend_d) * W,
            args.arguments.at<float>(KnownArgs::ascend_a)},
        vehicle_domain_from_string(args.arguments.at<std::string>(KnownArgs::vehicle_domain)));
}
