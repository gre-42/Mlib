#include "Create_Wing.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Actuators/Wing.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const float LIFT_COEFF_UNITS = N / squared(meters / s);
static const float ANGLE_COEFF_UNITS = N / degrees / squared(meters / s);
static const float DRAG_COEFF_UNITS = N / squared(meters / s);

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(VEHICLE);
DECLARE_OPTION(ANGLE_OF_ATTACK_NODE);
DECLARE_OPTION(BRAKE_ANGLE_NODE);

DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);

DECLARE_OPTION(ROTATION_X);
DECLARE_OPTION(ROTATION_Y);
DECLARE_OPTION(ROTATION_Z);

DECLARE_OPTION(FAC_V);
DECLARE_OPTION(FAC_C);

DECLARE_OPTION(LIFT_C);
DECLARE_OPTION(ANGLE_YZ);
DECLARE_OPTION(ANGLE_ZZ);

DECLARE_OPTION(DRAG_X);
DECLARE_OPTION(DRAG_Y);
DECLARE_OPTION(DRAG_Z);

DECLARE_OPTION(WING_ID);

LoadSceneUserFunction CreateWing::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*wing"
        "\\s+vehicle=([\\w+-.]+)"
        "(?:\\s+angle_of_attack_node=([\\w+-.]+))?"
        "(?:\\s+brake_angle_node=([\\w+-.]+))?"
        "\\s+position=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+fac_v=\\s*([ \\w+-.]+)"
        "\\s+fac_c=\\s*([ \\w+-.]+)"
        "\\s+lift_c=\\s*([\\w+-.]+)"
        "\\s+angle_yz=\\s*([\\w+-.]+)"
        "\\s+angle_zz=\\s*([\\w+-.]+)"
        "\\s+drag=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+wing_id=(\\d+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateWing(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateWing::CreateWing(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWing::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& vehicle_node = scene.get_node(match[VEHICLE].str());
    auto vehicle_rb = dynamic_cast<RigidBodyVehicle*>(&vehicle_node.get_absolute_movable());
    if (vehicle_rb == nullptr) {
        THROW_OR_ABORT("Car movable is not a rigid body");
    }
    FixedArray<double, 3> position{
        safe_stod(match[POSITION_X].str()),
        safe_stod(match[POSITION_Y].str()),
        safe_stod(match[POSITION_Z].str())};
    FixedArray<float, 3> rotation{
        safe_stof(match[ROTATION_X].str()) * degrees,
        safe_stof(match[ROTATION_Y].str()) * degrees,
        safe_stof(match[ROTATION_Z].str()) * degrees};
    size_t wing_id = safe_stoz(match[WING_ID].str());
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    Interp<float> fac{
        string_to_vector(match[FAC_V].str(), [](const std::string& s){return safe_stof(s) * kph;}),
        string_to_vector(match[FAC_C].str(), safe_stof),
        OutOfRangeBehavior::CLAMP};
    auto tp = vehicle_rb->wings_.insert({
        wing_id,
        std::make_unique<Wing>(
            TransformationMatrix<float, double, 3>{ r, position },
            fac,
            LIFT_COEFF_UNITS * safe_stof(match[LIFT_C].str()),
            ANGLE_COEFF_UNITS * safe_stof(match[ANGLE_YZ].str()),
            ANGLE_COEFF_UNITS * safe_stof(match[ANGLE_ZZ].str()),
            DRAG_COEFF_UNITS * FixedArray<float, 3>{
                safe_stof(match[DRAG_X].str()),
                safe_stof(match[DRAG_Y].str()),
                safe_stof(match[DRAG_Z].str())},
            0.f,
            0.f)});
    if (!tp.second) {
        THROW_OR_ABORT("Wing with ID \"" + std::to_string(wing_id) + "\" already exists");
    }
    if (match[ANGLE_OF_ATTACK_NODE].matched) {
        scene.get_node(match[ANGLE_OF_ATTACK_NODE].str()).set_relative_movable(
            observer_ptr<RelativeMovable>{&tp.first->second->angle_of_attack_movable, nullptr});
    }
    if (match[BRAKE_ANGLE_NODE].matched) {
        scene.get_node(match[BRAKE_ANGLE_NODE].str()).set_relative_movable(
            observer_ptr<RelativeMovable>{&tp.first->second->brake_angle_movable, nullptr});
    }
}
