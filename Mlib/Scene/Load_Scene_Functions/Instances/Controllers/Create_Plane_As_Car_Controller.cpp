#include "Create_Plane_As_Car_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Plane_As_Car_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Vehicle_Domain.hpp>
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
DECLARE_ARGUMENT(vehicle_domain);
}

const std::string CreatePlaneAsCarController::key = "create_plane_as_car_controller";

static float from_degrees(float v) {
    return v * degrees;
}

LoadSceneJsonUserFunction CreatePlaneAsCarController::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreatePlaneAsCarController(args.renderable_scene()).execute(args);
};

CreatePlaneAsCarController::CreatePlaneAsCarController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneAsCarController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node->get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Plane movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Plane controller already set");
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
    rb->vehicle_controller_ = std::make_unique<PlaneAsCarController>(
        *rb,
        tire_angles_map,
        vehicle_domain_from_string(args.arguments.at(KnownArgs::vehicle_domain)));
}
