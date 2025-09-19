#include "Connect_Trailer.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Record/Permanent_Point_Contact.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(car);
DECLARE_ARGUMENT(trailer);
}

const std::string ConnectTrailer::key = "connect_trailer";

LoadSceneJsonUserFunction ConnectTrailer::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ConnectTrailer(args.physics_scene()).execute(args);
};

ConnectTrailer::ConnectTrailer(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ConnectTrailer::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingBaseClassRef<SceneNode> car_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::car), DP_LOC);
    auto& car_rb = get_rigid_body_vehicle(car_node);
    DanglingBaseClassRef<SceneNode> trailer_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::trailer), DP_LOC);
    auto& trailer_rb = get_rigid_body_vehicle(trailer_node);
    physics_engine.permanent_contacts_.insert(std::make_unique<PermanentPointContact>(
        physics_engine.permanent_contacts_,
        car_node,
        trailer_node,
        car_rb.rbp_,
        trailer_rb.rbp_,
        car_rb.trailer_hitches_.get_position_male().casted<ScenePos>(),
        trailer_rb.trailer_hitches_.get_position_female().casted<ScenePos>()));
    car_rb.vehicle_controller().set_trailer({ trailer_rb.vehicle_controller(), CURRENT_SOURCE_LOCATION });
}
