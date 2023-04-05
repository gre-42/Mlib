#include "Create_Tank_Controller.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Tank_Controller.hpp>
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
DECLARE_OPTION(LEFT_TIRE_IDS);
DECLARE_OPTION(RIGHT_TIRE_IDS);
DECLARE_OPTION(DELTA_POWER);

LoadSceneUserFunction CreateTankController::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_tank_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+left_tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+right_tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+delta_power=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateTankController(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateTankController::CreateTankController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateTankController::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Tank movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Tank controller already set");
    }
    std::vector<size_t> left_tire_ids = string_to_vector(match[LEFT_TIRE_IDS].str(), safe_stoz);
    std::vector<size_t> right_tire_ids = string_to_vector(match[RIGHT_TIRE_IDS].str(), safe_stoz);
    rb->vehicle_controller_ = std::make_unique<TankController>(
        rb,
        left_tire_ids,
        right_tire_ids,
        safe_stof(match[DELTA_POWER].str()) * hp);
}
