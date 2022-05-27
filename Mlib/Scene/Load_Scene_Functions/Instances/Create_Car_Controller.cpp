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
DECLARE_OPTION(TIRE_IDS);
DECLARE_OPTION(TIRE_ANGLES);
DECLARE_OPTION(TIRE_ANGLE_PID_P);
DECLARE_OPTION(TIRE_ANGLE_PID_I);
DECLARE_OPTION(TIRE_ANGLE_PID_D);
DECLARE_OPTION(TIRE_ANGLE_PID_ALPHA);

LoadSceneUserFunction CreateCarController::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_car_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+tire_angles=((?:[\\w+-.]+)?(?:\\s+[\\w+-.]+)*)"
        "\\s+tire_angle_pid=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
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
    auto rb = dynamic_cast<RigidBodyVehicle*>(node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Car movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        throw std::runtime_error("Car controller already set");
    }
    std::vector<size_t> tire_ids = string_to_vector(match[TIRE_IDS].str(), safe_stoz);
    std::vector<float> tire_angles_deg = string_to_vector(match[TIRE_ANGLES].str(), safe_stof);
    if (tire_ids.size() != tire_angles_deg.size()) {
        throw std::runtime_error("Tire IDs and angles have different lengths");
    }
    std::map<size_t, float> tire_max_angles_map;
    for (size_t i = 0; i < tire_ids.size(); ++i) {
        if (!tire_max_angles_map.insert({ tire_ids[i], degrees * tire_angles_deg[i] }).second) {
            throw std::runtime_error("Duplicate tire ID");
        }
    }
    rb->vehicle_controller_ = std::make_unique<CarController>(
        rb,
        tire_max_angles_map,
        PidController<float, float>{
            safe_stof(match[TIRE_ANGLE_PID_P].str()),
            safe_stof(match[TIRE_ANGLE_PID_I].str()),
            safe_stof(match[TIRE_ANGLE_PID_D].str()),
            safe_stof(match[TIRE_ANGLE_PID_ALPHA].str())});
}
