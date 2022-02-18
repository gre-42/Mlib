#include "Create_Car_Controller.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controller.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateCarController::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_car_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+tire_angles=((?:[\\w+-.]+)?(?:\\s+[\\w+-.]+)*)$");
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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[1].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Car movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        throw std::runtime_error("Car controller already set");
    }
    std::vector<size_t> tire_ids = string_to_vector(match[2].str(), safe_stoz);
    std::vector<float> tire_angles_deg = string_to_vector(match[3].str(), safe_stof);
    if (tire_ids.size() != tire_angles_deg.size()) {
        throw std::runtime_error("Tire IDs and angles have different lengths");
    }
    std::map<size_t, float> tire_angles_map;
    for (size_t i = 0; i < tire_ids.size(); ++i) {
        if (!tire_angles_map.insert({ tire_ids[i], float(M_PI) / 180.f * tire_angles_deg[i] }).second) {
            throw std::runtime_error("Duplicate tire ID");
        }
    }
    rb->vehicle_controller_ = std::make_unique<CarController>(rb, tire_angles_map);
}
