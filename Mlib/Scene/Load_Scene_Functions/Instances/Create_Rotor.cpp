#include "Create_Rotor.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controller.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(VEHICLE);
DECLARE_OPTION(BLADES);

DECLARE_OPTION(VEHICLE_MOUNT_0_X);
DECLARE_OPTION(VEHICLE_MOUNT_0_Y);
DECLARE_OPTION(VEHICLE_MOUNT_0_Z);

DECLARE_OPTION(VEHICLE_MOUNT_1_X);
DECLARE_OPTION(VEHICLE_MOUNT_1_Y);
DECLARE_OPTION(VEHICLE_MOUNT_1_Z);

DECLARE_OPTION(BLADES_MOUNT_0_X);
DECLARE_OPTION(BLADES_MOUNT_0_Y);
DECLARE_OPTION(BLADES_MOUNT_0_Z);

DECLARE_OPTION(BLADES_MOUNT_1_X);
DECLARE_OPTION(BLADES_MOUNT_1_Y);
DECLARE_OPTION(BLADES_MOUNT_1_Z);

DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);

DECLARE_OPTION(ROTATION_X);
DECLARE_OPTION(ROTATION_Y);
DECLARE_OPTION(ROTATION_Z);

DECLARE_OPTION(ENGINE);
DECLARE_OPTION(POWER_2_LIFT);
DECLARE_OPTION(RPM);
DECLARE_OPTION(GRAVITY_CORRECTION);
DECLARE_OPTION(RADIUS);
DECLARE_OPTION(MAX_ALIGN_TO_GRAVITY);
DECLARE_OPTION(ALIGN_TO_GRAVITY_PID_P);
DECLARE_OPTION(ALIGN_TO_GRAVITY_PID_I);
DECLARE_OPTION(ALIGN_TO_GRAVITY_PID_D);
DECLARE_OPTION(ALIGN_TO_GRAVITY_PID_A);
DECLARE_OPTION(DRIFT_REDUCTION_FACTOR);
DECLARE_OPTION(DRIFT_REDUCTION_REFERENCE_VELOCITY);
DECLARE_OPTION(TIRE_ID);

LoadSceneUserFunction CreateRotor::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*rotor"
        "\\s+vehicle=([\\w+-.]+)"
        "(?:\\s+blades=([\\w+-.]+)"
        "\\s+vehicle_mount_0=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+vehicle_mount_1=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blades_mount_0=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blades_mount_1=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+position=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+engine=(\\w+)"
        "\\s+power2lift=([\\w+-.]+)"
        "\\s+rpm=([\\w+-.]+)"
        "(?:\\s+gravity_correction=(none|gimbal|move))?"
        "(?:\\s+radius=([\\w+-.]+))?"
        "(?:\\s+max_align_to_gravity=([\\w+-.]+))?"
        "(?:\\s+align_to_gravity_pid_p=([\\w+-.]+))?"
        "(?:\\s+align_to_gravity_pid_i=([\\w+-.]+))?"
        "(?:\\s+align_to_gravity_pid_d=([\\w+-.]+))?"
        "(?:\\s+align_to_gravity_pid_a=([\\w+-.]+))?"
        "(?:\\s+drift_reduction_factor=([\\w+-.]+))?"
        "(?:\\s+drift_reduction_reference_velocity=([\\w+-.]+))?"
        "\\s+tire_id=(\\d+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateRotor(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateRotor::CreateRotor(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRotor::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& vehicle_node = scene.get_node(match[VEHICLE].str());
    auto vehicle_rb = dynamic_cast<RigidBodyVehicle*>(&vehicle_node.get_absolute_movable());
    if (vehicle_rb == nullptr) {
        throw std::runtime_error("Car movable is not a rigid body");
    }
    FixedArray<float, 3> vehicle_mount_0(NAN);
    FixedArray<float, 3> vehicle_mount_1(NAN);
    FixedArray<float, 3> blades_mount_0(NAN);
    FixedArray<float, 3> blades_mount_1(NAN);
    RigidBodyVehicle* blades_rb = nullptr;
    std::string blades_node_name;
    if (match[BLADES].matched) {
        blades_node_name = match[BLADES].str();
        auto& blades_node = scene.get_node(blades_node_name);
        blades_rb = dynamic_cast<RigidBodyVehicle*>(&blades_node.get_absolute_movable());
        if (blades_rb == nullptr) {
            throw std::runtime_error("Blades movable is not a rigid body");
        }
        vehicle_mount_0 = FixedArray<float, 3>{
            safe_stof(match[VEHICLE_MOUNT_0_X].str()),
            safe_stof(match[VEHICLE_MOUNT_0_Y].str()),
            safe_stof(match[VEHICLE_MOUNT_0_Z].str())};
        vehicle_mount_1 = FixedArray<float, 3>{
            safe_stof(match[VEHICLE_MOUNT_1_X].str()),
            safe_stof(match[VEHICLE_MOUNT_1_Y].str()),
            safe_stof(match[VEHICLE_MOUNT_1_Z].str())};
        blades_mount_0 = FixedArray<float, 3>{
            safe_stof(match[BLADES_MOUNT_0_X].str()),
            safe_stof(match[BLADES_MOUNT_0_Y].str()),
            safe_stof(match[BLADES_MOUNT_0_Z].str())};
        blades_mount_1 = FixedArray<float, 3>{
            safe_stof(match[BLADES_MOUNT_1_X].str()),
            safe_stof(match[BLADES_MOUNT_1_Y].str()),
            safe_stof(match[BLADES_MOUNT_1_Z].str())};
    }
    FixedArray<double, 3> position{
        safe_stod(match[POSITION_X].str()),
        safe_stod(match[POSITION_Y].str()),
        safe_stod(match[POSITION_Z].str())};
    FixedArray<float, 3> rotation{
        safe_stof(match[ROTATION_X].str()) * degrees,
        safe_stof(match[ROTATION_Y].str()) * degrees,
        safe_stof(match[ROTATION_Z].str()) * degrees};
    std::string engine = match[ENGINE].str();
    float power2lift = safe_stof(match[POWER_2_LIFT].str()) * N / W;
    float w = safe_stof(match[RPM].str()) * rpm;
    GravityCorrection gravity_correction = match[GRAVITY_CORRECTION].matched
        ? gravity_correction_from_string(match[GRAVITY_CORRECTION].str())
        : GravityCorrection::NONE;
    float radius = match[RADIUS].matched
        ? safe_stof(match[RADIUS].str())
        : NAN;
    float max_align_to_gravity = match[MAX_ALIGN_TO_GRAVITY].matched
        ? safe_stof(match[MAX_ALIGN_TO_GRAVITY].str()) * degrees
        : NAN;
    size_t tire_id = safe_stoz(match[TIRE_ID].str());
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    PidController<float, float> pid{
        match[ALIGN_TO_GRAVITY_PID_P].matched ? safe_stof(match[ALIGN_TO_GRAVITY_PID_P].str()) : NAN,
        match[ALIGN_TO_GRAVITY_PID_I].matched ? safe_stof(match[ALIGN_TO_GRAVITY_PID_I].str()) : NAN,
        match[ALIGN_TO_GRAVITY_PID_D].matched ? safe_stof(match[ALIGN_TO_GRAVITY_PID_D].str()) : NAN,
        match[ALIGN_TO_GRAVITY_PID_A].matched ? safe_stof(match[ALIGN_TO_GRAVITY_PID_A].str()) : NAN};
    auto tp = vehicle_rb->rotors_.insert({
        tire_id,
        std::make_unique<Rotor>(
            engine,
            TransformationMatrix<float, double, 3>{ r, position },
            power2lift,
            w,
            gravity_correction,
            radius,
            max_align_to_gravity,
            pid,
            pid,
            match[DRIFT_REDUCTION_FACTOR].matched
                ? safe_stof(match[DRIFT_REDUCTION_FACTOR].str())
                : NAN,
            match[DRIFT_REDUCTION_REFERENCE_VELOCITY].matched
                ? safe_stof(match[DRIFT_REDUCTION_REFERENCE_VELOCITY].str()) * meters / s
                : NAN,
            vehicle_mount_0,
            vehicle_mount_1,
            blades_mount_0,
            blades_mount_1,
            blades_rb,
            blades_node_name,
            scene)});
    if (!tp.second) {
        throw std::runtime_error("Rotor with ID \"" + std::to_string(tire_id) + "\" already exists");
    }
}
