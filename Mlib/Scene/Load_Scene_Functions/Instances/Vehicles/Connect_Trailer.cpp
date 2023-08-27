#include "Connect_Trailer.hpp"
#include <Mlib/Argument_List.hpp>
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
    ConnectTrailer(args.renderable_scene()).execute(args);
};

ConnectTrailer::ConnectTrailer(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ConnectTrailer::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> car_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::car));
    auto car_rb = dynamic_cast<RigidBodyVehicle*>(&car_node->get_absolute_movable());
    if (car_rb == nullptr) {
        THROW_OR_ABORT("Car absolute movable is not a rigid body");
    }
    DanglingRef<SceneNode> trailer_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::trailer));
    auto trailer_rb = dynamic_cast<RigidBodyVehicle*>(&trailer_node->get_absolute_movable());
    if (trailer_rb == nullptr) {
        THROW_OR_ABORT("Trailer absolute movable is not a rigid body");
    }
    physics_engine.permanent_contacts_.insert(std::make_unique<PermanentPointContact>(
        physics_engine.permanent_contacts_,
        car_node,
        trailer_node,
        car_rb->rbi_.rbp_,
        trailer_rb->rbi_.rbp_,
        car_rb->trailer_hitches_.get_position_male().casted<double>(),
        trailer_rb->trailer_hitches_.get_position_female().casted<double>()));
    car_rb->vehicle_controller().set_trailer(trailer_rb->vehicle_controller());
}
