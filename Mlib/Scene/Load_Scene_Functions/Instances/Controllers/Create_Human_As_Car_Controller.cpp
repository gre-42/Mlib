#include "Create_Human_As_Car_Controller.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Human_As_Car_Controller.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(STEERING_MULTIPLIER);

const std::string CreateHumanAsCarController::key = "create_human_as_car_controller";

LoadSceneUserFunction CreateHumanAsCarController::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^node=([\\w+-.]+)"
        "\\s+steering_multiplier=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateHumanAsCarController(args.renderable_scene()).execute(match, args);
};

CreateHumanAsCarController::CreateHumanAsCarController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateHumanAsCarController::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Car movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Human controller already set");
    }
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&node.get_relative_movable());
    if (ypln == nullptr) {
        THROW_OR_ABORT("Relative movable is not a ypln");
    }
    rb->vehicle_controller_ = std::make_unique<HumanAsCarController>(
        rb,
        ypln,
        safe_stof(match[STEERING_MULTIPLIER].str()));
}
