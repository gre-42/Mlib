#include "Create_Plane_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Plane_Controllers/Plane_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Vehicle_Domain.hpp>
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
DECLARE_ARGUMENT(left_front_aileron_wing_ids);
DECLARE_ARGUMENT(right_front_aileron_wing_ids);
DECLARE_ARGUMENT(left_rear_aileron_wing_ids);
DECLARE_ARGUMENT(right_rear_aileron_wing_ids);
DECLARE_ARGUMENT(left_rudder_wing_ids);
DECLARE_ARGUMENT(right_rudder_wing_ids);
DECLARE_ARGUMENT(left_flap_wing_ids);
DECLARE_ARGUMENT(right_flap_wing_ids);
DECLARE_ARGUMENT(tire_ids);
DECLARE_ARGUMENT(tire_angles);
DECLARE_ARGUMENT(yaw_amount_to_tire_angle);
DECLARE_ARGUMENT(vehicle_domain);
}

const std::string CreatePlaneController::key = "create_plane_controller";

static float from_degrees(float v) {
    return v * degrees;
}

LoadSceneJsonUserFunction CreatePlaneController::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreatePlaneController(args.renderable_scene()).execute(args);
};

CreatePlaneController::CreatePlaneController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node->get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Plane movable is not a rigid body");
    }
    if (rb->plane_controller_ != nullptr) {
        THROW_OR_ABORT("Plane controller already set");
    }
    auto left_front_aileron_wing_ids  = args.arguments.at<std::vector<size_t>>(KnownArgs::left_front_aileron_wing_ids);
    auto right_front_aileron_wing_ids = args.arguments.at<std::vector<size_t>>(KnownArgs::right_front_aileron_wing_ids);
    auto left_rear_aileron_wing_ids   = args.arguments.at<std::vector<size_t>>(KnownArgs::left_rear_aileron_wing_ids);
    auto right_rear_aileron_wing_ids  = args.arguments.at<std::vector<size_t>>(KnownArgs::right_rear_aileron_wing_ids);
    auto left_rudder_wing_ids         = args.arguments.at<std::vector<size_t>>(KnownArgs::left_rudder_wing_ids);
    auto right_rudder_wing_ids        = args.arguments.at<std::vector<size_t>>(KnownArgs::right_rudder_wing_ids);
    auto left_flap_wing_ids           = args.arguments.at<std::vector<size_t>>(KnownArgs::left_flap_wing_ids);
    auto right_flap_wing_ids          = args.arguments.at<std::vector<size_t>>(KnownArgs::right_flap_wing_ids);

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
    rb->plane_controller_ = std::make_unique<PlaneController>(
        *rb,
        left_front_aileron_wing_ids,
        right_front_aileron_wing_ids,
        left_rear_aileron_wing_ids,
        right_rear_aileron_wing_ids,
        left_rudder_wing_ids,
        right_rudder_wing_ids,
        left_flap_wing_ids,
        right_flap_wing_ids,
        tire_angles_map,
        args.arguments.at<float>(KnownArgs::yaw_amount_to_tire_angle),
        vehicle_domain_from_string(args.arguments.at<std::string>(KnownArgs::vehicle_domain)));
}
