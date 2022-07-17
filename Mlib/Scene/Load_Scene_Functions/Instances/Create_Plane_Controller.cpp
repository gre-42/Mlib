#include "Create_Plane_Controller.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Plane_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Vehicle_Domain.hpp>
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
DECLARE_OPTION(LEFT_FRONT_AILERON_WING_IDS);
DECLARE_OPTION(RIGHT_FRONT_AILERON_WING_IDS);
DECLARE_OPTION(LEFT_REAR_AILERON_WING_IDS);
DECLARE_OPTION(RIGHT_REAR_AILERON_WING_IDS);
DECLARE_OPTION(LEFT_RUDDER_WING_IDS);
DECLARE_OPTION(RIGHT_RUDDER_WING_IDS);
DECLARE_OPTION(LEFT_FLAP_WING_IDS);
DECLARE_OPTION(RIGHT_FLAP_WING_IDS);
DECLARE_OPTION(TIRE_IDS);
DECLARE_OPTION(TIRE_ANGLES);
DECLARE_OPTION(YAW_AMOUNT_TO_TIRE_ANGLE);
DECLARE_OPTION(TURBINE_ID);
DECLARE_OPTION(VEHICLE_DOMAIN);

LoadSceneUserFunction CreatePlaneController::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_plane_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+left_front_aileron_wing_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+right_front_aileron_wing_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+left_rear_aileron_wing_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+right_rear_aileron_wing_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+left_rudder_wing_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+right_rudder_wing_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+left_flap_wing_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+right_flap_wing_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+tire_angles=((?:[\\w+-.]+)?(?:\\s+[\\w+-.]+)*)"
        "\\s+yaw_amount_to_tire_angle=([\\w+-.]+)"
        "\\s+turbine_id=(\\d+)"
        "\\s+vehicle_domain=(air|ground)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreatePlaneController(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreatePlaneController::CreatePlaneController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneController::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Plane movable is not a rigid body");
    }
    if (rb->plane_controller_ != nullptr) {
        throw std::runtime_error("Plane controller already set");
    }
    std::vector<size_t> left_front_aileron_wing_ids = string_to_vector(match[LEFT_FRONT_AILERON_WING_IDS].str(), safe_stoz);
    std::vector<size_t> right_front_aileron_wing_ids = string_to_vector(match[RIGHT_FRONT_AILERON_WING_IDS].str(), safe_stoz);
    std::vector<size_t> left_rear_aileron_wing_ids = string_to_vector(match[LEFT_REAR_AILERON_WING_IDS].str(), safe_stoz);
    std::vector<size_t> right_rear_aileron_wing_ids = string_to_vector(match[RIGHT_REAR_AILERON_WING_IDS].str(), safe_stoz);
    std::vector<size_t> left_rudder_wing_ids = string_to_vector(match[LEFT_RUDDER_WING_IDS].str(), safe_stoz);
    std::vector<size_t> right_rudder_wing_ids = string_to_vector(match[RIGHT_RUDDER_WING_IDS].str(), safe_stoz);
    std::vector<size_t> left_flap_wing_ids = string_to_vector(match[LEFT_FLAP_WING_IDS].str(), safe_stoz);
    std::vector<size_t> right_flap_wing_ids = string_to_vector(match[RIGHT_FLAP_WING_IDS].str(), safe_stoz);

    std::vector<size_t> tire_ids = string_to_vector(match[TIRE_IDS].str(), safe_stoz);
    std::vector<float> tire_angles_deg = string_to_vector(match[TIRE_ANGLES].str(), safe_stof);
    if (tire_ids.size() != tire_angles_deg.size()) {
        throw std::runtime_error("Tire IDs and angles have different lengths");
    }
    std::map<size_t, float> tire_angles_map;
    for (size_t i = 0; i < tire_ids.size(); ++i) {
        if (!tire_angles_map.insert({ tire_ids[i], degrees * tire_angles_deg[i] }).second) {
            throw std::runtime_error("Duplicate tire ID");
        }
    }
    rb->plane_controller_ = std::make_unique<PlaneController>(
        rb,
        left_front_aileron_wing_ids,
        right_front_aileron_wing_ids,
        left_rear_aileron_wing_ids,
        right_rear_aileron_wing_ids,
        left_rudder_wing_ids,
        right_rudder_wing_ids,
        left_flap_wing_ids,
        right_flap_wing_ids,
        tire_angles_map,
        safe_stof(match[YAW_AMOUNT_TO_TIRE_ANGLE].str()),
        safe_stoz(match[TURBINE_ID].str()),
        vehicle_domain_from_string(match[VEHICLE_DOMAIN].str()));
}
