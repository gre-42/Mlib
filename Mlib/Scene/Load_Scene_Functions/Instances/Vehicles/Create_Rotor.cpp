#include "Create_Rotor.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Actuators/Rotor.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(vehicle);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(engine);
DECLARE_ARGUMENT(delta_engine);
DECLARE_ARGUMENT(power2lift);
DECLARE_ARGUMENT(rpm);
DECLARE_ARGUMENT(gravity_correction);
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(blades);
DECLARE_ARGUMENT(max_align_to_gravity);
DECLARE_ARGUMENT(align_to_gravity_pid);
DECLARE_ARGUMENT(drift_reduction_factor);
DECLARE_ARGUMENT(drift_reduction_reference_velocity);
DECLARE_ARGUMENT(rotor_id);
}

namespace BladesArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(vehicle_mount_0);
DECLARE_ARGUMENT(vehicle_mount_1);
DECLARE_ARGUMENT(blades_mount_0);
DECLARE_ARGUMENT(blades_mount_1);
}
namespace PidArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(pid);
DECLARE_ARGUMENT(alpha);
}

const std::string CreateRotor::key = "rotor";

LoadSceneJsonUserFunction CreateRotor::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    if (args.arguments.contains(KnownArgs::blades)) {
        args.arguments.child(KnownArgs::blades).validate(BladesArgs::options);
    }
    if (args.arguments.contains(KnownArgs::align_to_gravity_pid)) {
        args.arguments.child(KnownArgs::align_to_gravity_pid).validate(PidArgs::options);
    }
    CreateRotor(args.renderable_scene()).execute(args);
};

CreateRotor::CreateRotor(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRotor::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& vehicle_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::vehicle));
    auto vehicle_rb = dynamic_cast<RigidBodyVehicle*>(&vehicle_node.get_absolute_movable());
    if (vehicle_rb == nullptr) {
        THROW_OR_ABORT("Car movable is not a rigid body");
    }
    FixedArray<float, 3> vehicle_mount_0(NAN);
    FixedArray<float, 3> vehicle_mount_1(NAN);
    FixedArray<float, 3> blades_mount_0(NAN);
    FixedArray<float, 3> blades_mount_1(NAN);
    RigidBodyVehicle* blades_rb = nullptr;
    std::string blades_node_name;
    if (args.arguments.contains(KnownArgs::blades)) {
        auto c = args.arguments.child(KnownArgs::blades);
        blades_node_name = c.at<std::string>(BladesArgs::node);
        auto& blades_node = scene.get_node(blades_node_name);
        blades_rb = dynamic_cast<RigidBodyVehicle*>(&blades_node.get_absolute_movable());
        if (blades_rb == nullptr) {
            THROW_OR_ABORT("Blades movable is not a rigid body");
        }
        vehicle_mount_0 = c.at<FixedArray<float, 3>>(BladesArgs::vehicle_mount_0);
        vehicle_mount_1 = c.at<FixedArray<float, 3>>(BladesArgs::vehicle_mount_1);
        blades_mount_0 = c.at<FixedArray<float, 3>>(BladesArgs::blades_mount_0);
        blades_mount_1 = c.at<FixedArray<float, 3>>(BladesArgs::blades_mount_1);
    }
    FixedArray<double, 3> position = args.arguments.at<FixedArray<double, 3>>(KnownArgs::position) * (double)meters;
    FixedArray<float, 3> rotation = args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation) * degrees;
    auto engine = args.arguments.at<std::string>(KnownArgs::engine);
    auto delta_engine = args.arguments.try_at<std::string>(KnownArgs::delta_engine);
    float power2lift = args.arguments.at<float>(KnownArgs::power2lift) * N / W;
    float w = args.arguments.at<float>(KnownArgs::rpm) * rpm;
    GravityCorrection gravity_correction = args.arguments.contains(KnownArgs::gravity_correction)
        ? gravity_correction_from_string(args.arguments.at<std::string>(KnownArgs::gravity_correction))
        : GravityCorrection::NONE;
    float radius = args.arguments.contains(KnownArgs::radius)
        ? args.arguments.at<float>(KnownArgs::radius) * meters
        : NAN;
    float max_align_to_gravity = args.arguments.contains(KnownArgs::max_align_to_gravity)
        ? args.arguments.at<float>(KnownArgs::max_align_to_gravity) * degrees
        : NAN;
    size_t rotor_id = args.arguments.at<size_t>(KnownArgs::rotor_id);
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    auto pid_child = args.arguments.try_get_child(KnownArgs::align_to_gravity_pid);
    FixedArray<float, 3> pid_params = pid_child.has_value()
        ? pid_child->at<FixedArray<float, 3>>(PidArgs::pid)
        : fixed_nans<float, 3>();
    PidController<float, float> pid{
        pid_params(0),
        pid_params(1),
        pid_params(2),
        pid_child.has_value() ? pid_child->at<float>(PidArgs::alpha) : NAN};
    auto tp = vehicle_rb->rotors_.insert({
        rotor_id,
        std::make_unique<Rotor>(
            engine,
            delta_engine,
            TransformationMatrix<float, double, 3>{ r, position },
            power2lift,
            w,
            gravity_correction,
            radius,
            max_align_to_gravity,
            pid,
            pid,
            args.arguments.contains(KnownArgs::drift_reduction_factor)
                ? args.arguments.at<float>(KnownArgs::drift_reduction_factor)
                : NAN,
            args.arguments.contains(KnownArgs::drift_reduction_reference_velocity)
                ? args.arguments.at<float>(KnownArgs::drift_reduction_reference_velocity) * meters / s
                : NAN,
            vehicle_mount_0,
            vehicle_mount_1,
            blades_mount_0,
            blades_mount_1,
            blades_rb,
            blades_node_name)});
    if (!tp.second) {
        THROW_OR_ABORT("Rotor with ID \"" + std::to_string(rotor_id) + "\" already exists");
    }
}
