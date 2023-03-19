#include "Create_Heli_Controller.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Heli_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Vehicle_Domain.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(TIRE_IDS);
DECLARE_OPTION(TIRE_ANGLES);
DECLARE_OPTION(MAIN_ROTOR_ID);
DECLARE_OPTION(PITCH_MULTIPLIER);
DECLARE_OPTION(YAW_MULTIPLIER);
DECLARE_OPTION(ROLL_MULTIPLIER);
DECLARE_OPTION(ASCEND_P);
DECLARE_OPTION(ASCEND_I);
DECLARE_OPTION(ASCEND_D);
DECLARE_OPTION(ASCEND_A);
DECLARE_OPTION(VEHICLE_DOMAIN);

LoadSceneUserFunction CreateHeliController::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_heli_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+tire_angles=((?:[\\w+-.]+)?(?:\\s+[\\w+-.]+)*)"
        "\\s+main_rotor_id=(\\d+)"
        "\\s+pitch_multiplier=([\\w+-.]+)"
        "\\s+yaw_multiplier=([\\w+-.]+)"
        "\\s+roll_multiplier=([\\w+-.]+)"
        "\\s+ascend_p=([\\w+-.]+)"
        "\\s+ascend_i=([\\w+-.]+)"
        "\\s+ascend_d=([\\w+-.]+)"
        "\\s+ascend_a=([\\w+-.]+)"
        "\\s+vehicle_domain=(air|ground)$");
    Mlib::re::smatch match;
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Heli movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Heli controller already set");
    }
    std::vector<size_t> tire_ids = string_to_vector(match[TIRE_IDS].str(), safe_stoz);
    std::vector<float> tire_angles_deg = string_to_vector(match[TIRE_ANGLES].str(), safe_stof);
    if (tire_ids.size() != tire_angles_deg.size()) {
        THROW_OR_ABORT("Tire IDs and angles have different lengths");
    }
    std::map<size_t, float> tire_angles_map;
    for (size_t i = 0; i < tire_ids.size(); ++i) {
        if (!tire_angles_map.insert({ tire_ids[i], degrees * tire_angles_deg[i] }).second) {
            THROW_OR_ABORT("Duplicate tire ID");
        }
    }
    rb->vehicle_controller_ = std::make_unique<HeliController>(
        rb,
        tire_angles_map,
        safe_stoz(match[MAIN_ROTOR_ID].str()),
        FixedArray<float, 3>{
            safe_stof(match[PITCH_MULTIPLIER].str()),
            safe_stof(match[YAW_MULTIPLIER].str()) * W,
            safe_stof(match[ROLL_MULTIPLIER].str())},
        PidController<double, double>{
            safe_stof(match[ASCEND_P].str()) * W,
            safe_stof(match[ASCEND_I].str()) * W,
            safe_stof(match[ASCEND_D].str()) * W,
            safe_stof(match[ASCEND_A].str())},
        vehicle_domain_from_string(match[VEHICLE_DOMAIN].str()));
}
