#include "Create_Abs_Key_Binding.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(ROLE);
DECLARE_OPTION(NODE);
DECLARE_OPTION(FORCE_X);
DECLARE_OPTION(FORCE_Y);
DECLARE_OPTION(FORCE_Z);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);
DECLARE_OPTION(ROTATE_X);
DECLARE_OPTION(ROTATE_Y);
DECLARE_OPTION(ROTATE_Z);
DECLARE_OPTION(CAR_SURFACE_POWER);
DECLARE_OPTION(MAX_VELOCITY);
DECLARE_OPTION(TIRE_ID);
DECLARE_OPTION(TIRE_ANGLE_VELOCITIES);
DECLARE_OPTION(TIRE_ANGLES);
DECLARE_OPTION(TIRES_Z_X);
DECLARE_OPTION(TIRES_Z_Y);
DECLARE_OPTION(TIRES_Z_Z);
DECLARE_OPTION(WANTS_TO_JUMP);
DECLARE_OPTION(WANTS_TO_GRIND);
DECLARE_OPTION(FLY_FORWARD_FACTOR);

LoadSceneUserFunction CreateAbsKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*abs_key_binding"
        "\\s+id=([\\w+-.]+)"
        "\\s+role=([\\w+-.]+)"

        "\\s+node=([\\w+-.]+)"
        "(?:\\s+force=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+rotate=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+car_surface_power=([\\w+-.]+))?"
        "(?:\\s+max_velocity=([\\w+-.]+))?"
        "(?:\\s+tire_id=(\\d+)"
        "\\s+tire_angle_velocities=([ \\w+-.]+)"
        "\\s+tire_angles=([ \\w+-.]+))?"
        "(?:\\s+tires_z=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+wants_to_jump=([ \\w+-.]+))?"
        "(?:\\s+wants_to_grind=([ \\w+-.]+))?"
        "(?:\\s+fly_forward_factor=([ \\w+-.]+))?$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateAbsKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateAbsKeyBinding::CreateAbsKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAbsKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(match[NODE].str()).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    key_bindings.add_absolute_movable_key_binding(AbsoluteMovableKeyBinding{
        .id = match[ID].str(),
        .role = match[ROLE].str(),
        .node = &scene.get_node(match[NODE].str()),
        .force = {
            .vector = {
                match[FORCE_X].matched ? safe_stof(match[FORCE_X].str()) * N : 0.f,
                match[FORCE_Y].matched ? safe_stof(match[FORCE_Y].str()) * N : 0.f,
                match[FORCE_Z].matched ? safe_stof(match[FORCE_Z].str()) * N : 0.f},
            .position = {
                match[POSITION_X].matched ? safe_stof(match[POSITION_X].str()) * meters : rb->rbi_.rbp_.com_(0),
                match[POSITION_Y].matched ? safe_stof(match[POSITION_Y].str()) * meters : rb->rbi_.rbp_.com_(1),
                match[POSITION_Z].matched ? safe_stof(match[POSITION_Z].str()) * meters : rb->rbi_.rbp_.com_(2)}},
        .rotate = {
            match[ROTATE_X].matched ? safe_stof(match[ROTATE_X].str()) : 0.f,
            match[ROTATE_Y].matched ? safe_stof(match[ROTATE_Y].str()) : 0.f,
            match[ROTATE_Z].matched ? safe_stof(match[ROTATE_Z].str()) : 0.f},
        .car_surface_power = match[CAR_SURFACE_POWER].matched ? safe_stof(match[CAR_SURFACE_POWER].str()) * W : std::optional<float>(),
        .max_velocity = match[MAX_VELOCITY].matched ? safe_stof(match[MAX_VELOCITY].str()) * meters / s : INFINITY,
        .tire_id = match[TIRE_ID].matched ? safe_stoz(match[TIRE_ID].str()) : SIZE_MAX,
        .tire_angle_interp = Interp<float>{
            string_to_vector(match[TIRE_ANGLE_VELOCITIES].str(), safe_stof),
            string_to_vector(match[TIRE_ANGLES].str(), safe_stof),
            OutOfRangeBehavior::CLAMP},
        .tires_z = {
            match[TIRES_Z_X].matched ? safe_stof(match[TIRES_Z_X].str()) : 0.f,
            match[TIRES_Z_Y].matched ? safe_stof(match[TIRES_Z_Y].str()) : 0.f,
            match[TIRES_Z_Z].matched ? safe_stof(match[TIRES_Z_Z].str()) : 0.f},
        .wants_to_jump = match[WANTS_TO_JUMP].matched
            ? safe_stob(match[WANTS_TO_JUMP].str())
            : std::optional<bool>(),
        .wants_to_grind = match[WANTS_TO_GRIND].matched
            ? safe_stob(match[WANTS_TO_GRIND].str())
            : std::optional<bool>(),
        .fly_forward_factor = match[FLY_FORWARD_FACTOR].matched
            ? safe_stof(match[FLY_FORWARD_FACTOR].str()) * N
            : std::optional<float>()});
}
