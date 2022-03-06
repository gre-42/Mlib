#include "Create_Heli_Controller.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Heli_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Vehicle_Domain.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateHeliController::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_heli_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+tire_angles=((?:[\\w+-.]+)?(?:\\s+[\\w+-.]+)*)"
        "\\s+main_rotor_id=(\\d+)"
        "\\s+tail_rotor_id=(\\d+)"
        "\\s+pitch_multiplier=([\\w+-.]+)"
        "\\s+yaw_multiplier=([\\w+-.]+)"
        "\\s+roll_multiplier=([\\w+-.]+)"
        "\\s+ascend_p=([\\w+-.]+)"
        "\\s+ascend_i=([\\w+-.]+)"
        "\\s+ascend_d=([\\w+-.]+)"
        "\\s+ascend_a=([\\w+-.]+)"
        "\\s+vehicle_domain=(air|ground)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateHeliController(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateHeliController::CreateHeliController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateHeliController::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[1].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Heli movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        throw std::runtime_error("Heli controller already set");
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
    rb->vehicle_controller_ = std::make_unique<HeliController>(
        rb,
        tire_angles_map,
        safe_stoz(match[4].str()),            // main_rotor_id
        safe_stoz(match[5].str()),            // tail_rotor_id
        FixedArray<float, 3>{
            safe_stof(match[6].str()),        // pitch_multiplier
            safe_stof(match[7].str()),        // yaw_multiplier
            safe_stof(match[8].str())},       // roll_multiplier
        PidController<float, float>{
            safe_stof(match[9].str()),        // p
            safe_stof(match[10].str()),       // i
            safe_stof(match[11].str()),       // d
            safe_stof(match[12].str())},      // a
        vehicle_domain_from_string(match[13].str()));
}
