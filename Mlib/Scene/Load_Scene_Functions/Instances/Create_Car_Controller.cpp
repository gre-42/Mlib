#include "Create_Car_Controller.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controller.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(FRONT_TIRE_IDS);
DECLARE_OPTION(MAX_TIRE_ANGLE);

LoadSceneUserFunction CreateCarController::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_car_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+front_tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+max_tire_angle=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateCarController(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateCarController::CreateCarController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCarController::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Car movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        throw std::runtime_error("Car controller already set");
    }
    rb->vehicle_controller_ = std::make_unique<CarController>(
        rb,
        string_to_vector(match[FRONT_TIRE_IDS].str(), safe_stoz),
        safe_stof(match[MAX_TIRE_ANGLE].str()) * degrees,
        physics_engine);
}
