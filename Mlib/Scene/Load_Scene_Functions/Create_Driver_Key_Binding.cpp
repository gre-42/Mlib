#include "Create_Driver_Key_Binding.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);

DECLARE_OPTION(KEY);
DECLARE_OPTION(GAMEPAD_BUTTON);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN);

DECLARE_OPTION(KEY2);
DECLARE_OPTION(GAMEPAD_BUTTON2);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS2);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN2);

DECLARE_OPTION(NOT_KEY);
DECLARE_OPTION(NOT_GAMEPAD_BUTTON);
DECLARE_OPTION(NOT_JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(NOT_JOYSTICK_DIGITAL_AXIS_SIGN);

DECLARE_OPTION(SELECT_NEXT_OPPONENT);
DECLARE_OPTION(SELECT_NEXT_VEHICLE);

LoadSceneInstanceFunction::UserFunction CreateDriverKeyBinding::user_function = [](const UserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*player_key_binding"
        "\\s+node=([\\w+-.]+)"

        "(?:\\s+key=([\\w+-.]+))?"
        "(?:\\s+gamepad_button=([\\w+-.]+))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]+)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?"

        "(?:\\s+key2=([\\w+-.]+))?"
        "(?:\\s+gamepad_button2=([\\w+-.]+))?"
        "(?:\\s+joystick_digital_axis2=([\\w+-.]+)"
        "\\s+joystick_digital_axis_sign2=([\\w+-.]+))?"

        "(?:\\s+not_key=([\\w+-.]+))?"
        "(?:\\s+not_gamepad_button=([\\w+-.]+))?"
        "(?:\\s+not_joystick_digital_axis=([\\w+-.]+)"
        "\\s+not_joystick_digital_axis_sign=([\\w+-.]+))?"
        
        "(\\s+select_next_opponent)?"
        "(\\s+select_next_vehicle)?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateDriverKeyBinding(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateDriverKeyBinding::CreateDriverKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDriverKeyBinding::execute(
    const std::smatch& match,
    const UserFunctionArgs& args)
{
    SceneNode* node = scene.get_node(match[NODE].str());
    BaseKeyCombination combo = {
        .key_bindings = {
            BaseKeyBinding{
                .key = match[KEY].str(),
                .gamepad_button = match[GAMEPAD_BUTTON].str(),
                .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
                .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].matched
                    ? safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())
                    : 0.f }},
        .not_key_binding = BaseKeyBinding{
            .key = match[NOT_KEY].str(),
            .gamepad_button = match[NOT_GAMEPAD_BUTTON].str(),
            .joystick_axis = match[NOT_JOYSTICK_DIGITAL_AXIS].str(),
            .joystick_axis_sign = match[NOT_JOYSTICK_DIGITAL_AXIS_SIGN].matched
                ? safe_stof(match[NOT_JOYSTICK_DIGITAL_AXIS_SIGN].str())
                : 0.f }};
    if (match[KEY2].matched ||
        match[GAMEPAD_BUTTON2].matched ||
        match[JOYSTICK_DIGITAL_AXIS2].matched)
    {
        combo.key_bindings.insert(BaseKeyBinding{
            .key = match[KEY2].str(),
            .gamepad_button = match[GAMEPAD_BUTTON2].str(),
            .joystick_axis = match[JOYSTICK_DIGITAL_AXIS2].str(),
            .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN2].matched
                ? safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN2].str())
                : 0.f });
    }
    auto& kb = key_bindings.add_player_key_binding(PlayerKeyBinding{
        .base_combo = combo,
        .node = node,
        .select_next_opponent = match[SELECT_NEXT_OPPONENT].matched,
        .select_next_vehicle = match[SELECT_NEXT_VEHICLE].matched});
    auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[1].str())->get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    if (rb->driver_ == nullptr) {
        throw std::runtime_error("Rigid body has no driver");
    }
    auto player = dynamic_cast<Player*>(rb->driver_);
    if (player == nullptr) {
        throw std::runtime_error("Driver is not player");
    }
    player->append_delete_externals(
        nullptr,
        [&kbs=key_bindings, &kb](){kbs.delete_player_key_binding(kb);});
}
