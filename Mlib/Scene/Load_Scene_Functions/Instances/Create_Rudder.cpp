#include "Create_Rudder.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

static const float DRAG_COEFF_FAC = N / squared(meters / s);
static const float ANGLE_COEFF_FAC = N / degrees / squared(meters / s);

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(VEHICLE);

DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);

DECLARE_OPTION(ROTATION_X);
DECLARE_OPTION(ROTATION_Y);
DECLARE_OPTION(ROTATION_Z);

DECLARE_OPTION(FORCE);
DECLARE_OPTION(DRAG_X);
DECLARE_OPTION(DRAG_Y);
DECLARE_OPTION(DRAG_Z);

DECLARE_OPTION(RUDDER_ID);

LoadSceneUserFunction CreateRudder::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*rudder"
        "\\s+vehicle=([\\w+-.]+)"
        "\\s+position=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+force=\\s*([\\w+-.]+)"
        "\\s+drag=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rudder_id=(\\d+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateRudder(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateRudder::CreateRudder(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRudder::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& vehicle_node = scene.get_node(match[VEHICLE].str());
    auto vehicle_rb = dynamic_cast<RigidBodyVehicle*>(&vehicle_node.get_absolute_movable());
    if (vehicle_rb == nullptr) {
        throw std::runtime_error("Car movable is not a rigid body");
    }
    FixedArray<double, 3> position{
        safe_stod(match[POSITION_X].str()),
        safe_stod(match[POSITION_Y].str()),
        safe_stod(match[POSITION_Z].str())};
    FixedArray<float, 3> rotation{
        safe_stof(match[ROTATION_X].str()) * degrees,
        safe_stof(match[ROTATION_Y].str()) * degrees,
        safe_stof(match[ROTATION_Z].str()) * degrees};
    size_t rudder_id = safe_stoz(match[RUDDER_ID].str());
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    auto tp = vehicle_rb->rudders_.insert({
        rudder_id,
        std::make_unique<Rudder>(
            TransformationMatrix<float, double, 3>{ r, position },
            0.f,
            ANGLE_COEFF_FAC * safe_stof(match[FORCE].str()),
            DRAG_COEFF_FAC * FixedArray<float, 3>{
                safe_stof(match[DRAG_X].str()),
                safe_stof(match[DRAG_Y].str()),
                safe_stof(match[DRAG_Z].str())})});
    if (!tp.second) {
        throw std::runtime_error("Rudder with ID \"" + std::to_string(rudder_id) + "\" already exists");
    }
}
