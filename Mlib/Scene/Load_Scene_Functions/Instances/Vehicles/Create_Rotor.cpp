#include "Create_Rotor.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
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
DECLARE_ARGUMENT(blades);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(engine);
DECLARE_ARGUMENT(delta_engine);
DECLARE_ARGUMENT(power2lift);
DECLARE_ARGUMENT(rpm);
DECLARE_ARGUMENT(gravity_correction);
DECLARE_ARGUMENT(radius);
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
    DanglingRef<SceneNode> vehicle_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::vehicle), DP_LOC);
    auto& vehicle_rb = get_rigid_body_vehicle(vehicle_node);
    FixedArray<float, 3> vehicle_mount_0(NAN);
    FixedArray<float, 3> vehicle_mount_1(NAN);
    FixedArray<float, 3> blades_mount_0(NAN);
    FixedArray<float, 3> blades_mount_1(NAN);
    RigidBodyVehicle* blades_rb = nullptr;
    if (args.arguments.contains(KnownArgs::blades)) {
        auto c = args.arguments.child(KnownArgs::blades);
        auto blades_node_name = c.at<VariableAndHash<std::string>>(BladesArgs::node);
        DanglingRef<SceneNode> blades_node = scene.get_node(blades_node_name, DP_LOC);
        blades_rb = &get_rigid_body_vehicle(blades_node);
        vehicle_mount_0 = c.at<UFixedArray<float, 3>>(BladesArgs::vehicle_mount_0);
        vehicle_mount_1 = c.at<UFixedArray<float, 3>>(BladesArgs::vehicle_mount_1);
        blades_mount_0 = c.at<UFixedArray<float, 3>>(BladesArgs::blades_mount_0);
        blades_mount_1 = c.at<UFixedArray<float, 3>>(BladesArgs::blades_mount_1);
    }
    FixedArray<ScenePos, 3> position = args.arguments.at<UFixedArray<ScenePos, 3>>(KnownArgs::position) * (ScenePos)meters;
    FixedArray<float, 3> rotation = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotation) * degrees;
    auto engine = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::engine);
    auto delta_engine = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::delta_engine);
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
    auto rotor_id = args.arguments.at<size_t>(KnownArgs::rotor_id);
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    auto pid_child = args.arguments.try_get_child(KnownArgs::align_to_gravity_pid);
    FixedArray<float, 3> pid_params = pid_child.has_value()
        ? pid_child->at<UFixedArray<float, 3>>(PidArgs::pid)
        : fixed_nans<float, 3>();
    PidController<float, float> pid{
        pid_params(0),
        pid_params(1),
        pid_params(2),
        pid_child.has_value() ? pid_child->at<float>(PidArgs::alpha) : NAN};
    vehicle_rb.rotors_.add(
        rotor_id,
        std::make_unique<Rotor>(
            engine,
            delta_engine,
            TransformationMatrix<float, ScenePos, 3>{ r, position },
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
                ? args.arguments.at<float>(KnownArgs::drift_reduction_reference_velocity) * kph
                : NAN,
            vehicle_mount_0,
            vehicle_mount_1,
            blades_mount_0,
            blades_mount_1,
            (blades_rb == nullptr) ? nullptr : &blades_rb->rbp_));
}
