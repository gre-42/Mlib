#include "Set_Actuator_Count.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(vehicle);
DECLARE_ARGUMENT(tires);
DECLARE_ARGUMENT(rotors);
DECLARE_ARGUMENT(wings);
}

SetActuatorCount::SetActuatorCount(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetActuatorCount::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto vehicle = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::vehicle);
    auto rb = get_rigid_body_vehicle(scene.get_node(vehicle, CURRENT_SOURCE_LOCATION).get(), CURRENT_SOURCE_LOCATION);
    if (auto tires = args.arguments.try_at<size_t>(KnownArgs::tires); tires.has_value()) {
        rb->tires_.resize(*tires);
    }
    if (auto rotors = args.arguments.try_at<size_t>(KnownArgs::rotors); rotors.has_value()) {
        rb->rotors_.resize(*rotors);
    }
    if (auto wings = args.arguments.try_at<size_t>(KnownArgs::wings); wings.has_value()) {
        rb->wings_.resize(*wings);
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_actuator_count",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetActuatorCount{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
