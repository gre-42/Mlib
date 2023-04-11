#include "Create_Car_Controller_Key_Binding.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(ROLE);

DECLARE_OPTION(PLAYER);
DECLARE_OPTION(NODE);

DECLARE_OPTION(SURFACE_POWER);
DECLARE_OPTION(TIRE_ANGLE_VELOCITIES);
DECLARE_OPTION(TIRE_ANGLES);
DECLARE_OPTION(ASCEND_VELOCITY);

const std::string CreateCarControllerKeyBinding::key = "car_controller_key_binding";

LoadSceneUserFunction CreateCarControllerKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^id=([\\w+-.]+)"
        "\\s+role=([\\w+-.]+)"

        "(?:\\s+player=([\\w+-.]+))?"
        "\\s+node=([\\w+-.]+)"

        "(?:\\s+surface_power=([\\w+-.]+))?"
        "(?:\\s+tire_angle_velocities=([ \\w+-.]+)"
        "\\s+tire_angles=([ \\w+-.]+))?"
        "(?:\\s+ascend_velocity=([\\w+-.]+))?$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateCarControllerKeyBinding(args.renderable_scene()).execute(match, args);
};

CreateCarControllerKeyBinding::CreateCarControllerKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

float stov(const std::string& str) {
    return safe_stof(str) * kph;
}

float stoa(const std::string& str) {
    return safe_stof(str) * degrees;
}

void CreateCarControllerKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto& kb = key_bindings.add_car_controller_key_binding(CarControllerKeyBinding{
        .id = match[ID].str(),
        .role = match[ROLE].str(),
        .node = &node,
        .surface_power = match[SURFACE_POWER].matched
            ? safe_stof(match[SURFACE_POWER].str()) * W
            : std::optional<float>(),
        .tire_angle_interp = match[TIRE_ANGLE_VELOCITIES].matched
            ? Interp<float>{
                string_to_vector(match[TIRE_ANGLE_VELOCITIES].str(), stov),
                string_to_vector(match[TIRE_ANGLES].str(), stoa),
                OutOfRangeBehavior::CLAMP}
            : std::optional<Interp<float>>(),
        .ascend_velocity = match[ASCEND_VELOCITY].matched
            ? stov(match[ASCEND_VELOCITY].str())
            : std::optional<float>()});
    if (match[PLAYER].matched) {
        players.get_player(match[PLAYER].str())
        .append_delete_externals(
            &node,
            [&kbs=key_bindings, &kb](){
                kbs.delete_car_controller_key_binding(kb);
            }
        );
    }
}
