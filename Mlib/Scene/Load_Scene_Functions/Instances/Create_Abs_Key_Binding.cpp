#include "Create_Abs_Key_Binding.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateAbsKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*abs_key_binding"
        "\\s+node=([\\w+-.]+)"
        "\\s+key=([\\w+-.]+)"
        "(?:\\s+gamepad_button=([\\w+-.]+))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]+)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?"
        "(?:\\s+force=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+rotate=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+car_surface_power=([\\w+-.]+))?"
        "(?:\\s+max_velocity=([\\w+-.]+))?"
        "(?:\\s+tire_id=(\\d+)"
        "\\s+tire_angle_velocities=([ \\w+-.]+)"
        "\\s+tire_angles=([ \\w+-.]+))?"
        "(?:\\s+tires_z=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?$");
    std::smatch match;
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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[1].str())->get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    key_bindings.add_absolute_movable_key_binding(AbsoluteMovableKeyBinding{
        .base_key = {
            .key = match[2].str(),
            .gamepad_button = match[3].str(),
            .joystick_axis = match[4].str(),
            .joystick_axis_sign = match[5].str().empty() ? 0 : safe_stof(match[5].str())},
        .node = scene.get_node(match[1].str()),
        .force = {
            .vector = {
                match[6].str().empty() ? 0.f : safe_stof(match[6].str()),
                match[7].str().empty() ? 0.f : safe_stof(match[7].str()),
                match[8].str().empty() ? 0.f : safe_stof(match[8].str())},
            .position = {
                match[9].str().empty() ? rb->rbi_.rbp_.com_(0) : safe_stof(match[9].str()),
                match[10].str().empty() ? rb->rbi_.rbp_.com_(1) : safe_stof(match[10].str()),
                match[11].str().empty() ? rb->rbi_.rbp_.com_(2) : safe_stof(match[11].str())}},
        .rotate = {
            match[12].str().empty() ? 0.f : safe_stof(match[12].str()),
            match[13].str().empty() ? 0.f : safe_stof(match[13].str()),
            match[14].str().empty() ? 0.f : safe_stof(match[14].str())},
        .car_surface_power = !match[15].matched ? std::optional<float>() : safe_stof(match[15].str()),
        .max_velocity = match[16].str().empty() ? INFINITY : safe_stof(match[16].str()),
        .tire_id = match[17].str().empty() ? SIZE_MAX : safe_stoi(match[17].str()),
        .tire_angle_interp = Interp<float>{
            string_to_vector(match[18].str(), safe_stof),
            string_to_vector(match[19].str(), safe_stof),
            OutOfRangeBehavior::CLAMP},
        .tires_z = {
            match[20].str().empty() ? 0.f : safe_stof(match[20].str()),
            match[21].str().empty() ? 0.f : safe_stof(match[21].str()),
            match[22].str().empty() ? 0.f : safe_stof(match[22].str())}});
}
