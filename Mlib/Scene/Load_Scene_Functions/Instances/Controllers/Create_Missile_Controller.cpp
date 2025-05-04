#include "Create_Missile_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Missile_Controller.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(wing_controllers);
DECLARE_ARGUMENT(engine);
}

namespace MWC {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(i);
DECLARE_ARGUMENT(angle_of_attack);
DECLARE_ARGUMENT(normal_angle);
DECLARE_ARGUMENT(antiroll_angle);
}

namespace Mlib {
void from_json(const nlohmann::json& j, MissileWingController& c) {
    JsonView jv{ j };
    jv.validate(MWC::options);
    j.at(MWC::i).get_to(c.i);
    float angle_of_attack = jv.at<float>(MWC::angle_of_attack) * degrees;
    float normal_angle = jv.at<float>(MWC::normal_angle) * degrees;
    c.gain = angle_of_attack * FixedArray<float, 2>{ std::cos(normal_angle), std::sin(normal_angle) };
    c.antiroll_angle = jv.at<float>(MWC::antiroll_angle) * degrees;
}
}

const std::string CreateMissileController::key = "create_missile_controller";

LoadSceneJsonUserFunction CreateMissileController::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateMissileController(args.renderable_scene()).execute(args);
};

CreateMissileController::CreateMissileController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateMissileController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    if (rb.missile_controller_ != nullptr) {
        THROW_OR_ABORT("Missile controller already set");
    }
    rb.missile_controller_ = std::make_unique<MissileController>(
        rb,
        args.arguments.at<std::vector<MissileWingController>>(KnownArgs::wing_controllers),
        VariableAndHash{ args.arguments.at<std::string>(KnownArgs::engine) });
}
