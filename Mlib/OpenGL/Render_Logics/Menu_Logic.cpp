#include "Menu_Logic.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/OpenGL/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/OpenGL/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/OpenGL/Key_Bindings/Make_Key_Binding.hpp>
#include <Mlib/OpenGL/Ui/Button_States.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <cstddef>
#include <sstream>
#include <stdexcept>

namespace Mlib {

static const auto& g = make_gamepad_button;
using S = VariableAndHash<std::string>;

class MenuLogicKeys {
public:
    explicit MenuLogicKeys(ButtonStates& button_states)
        : start{
            {button_states, key_configurations, 0, "escape", ""},
            {button_states, key_configurations, 1, "escape", ""} }
    {
        key_configurations
            .lock_exclusive_for(std::chrono::seconds(2), "Key configurations")
            ->insert(0, "escape", {{{BaseKeyBinding{
                .key = S("ESCAPE"),
                .gamepad_button = g(0, "START"),
                .tap_button = g(0, "ESCAPE")}}}});
        key_configurations
            .lock_exclusive_for(std::chrono::seconds(2), "Key configurations")
            ->insert(1, "escape", {{{BaseKeyBinding{
                .gamepad_button = g(1, "START"),
                .tap_button = g(1, "ESCAPE")}}}});
    }
    std::vector<ButtonPress> start;
private:
    LockableKeyConfigurations key_configurations;
};

}

using namespace Mlib;

MenuLogic::MenuLogic(
    MenuUserClass &user_object)
    : user_object_{user_object}
    , keys_{ std::make_unique<MenuLogicKeys>(user_object.button_states) }
{}

MenuLogic::~MenuLogic() = default;

void MenuLogic::handle_events() {
    LOG_FUNCTION("FlyingCameraLogic::render");
    for (auto [user_id, start] : tenumerate<uint32_t>(keys_->start)) {
        if (start.keys_pressed()) {
            auto& ui_focuses = user_object_.ui_focuses[user_id];
            auto& focuses = ui_focuses.focuses;
            if (ui_focuses.editing()) {
                continue;
            }
            std::scoped_lock lock{focuses.mutex};
            if (focuses.has_focus(Focus::MENU_ANY)) {
                if (focuses.size() > 1) {
                    if (!focuses.has_focus(Focus::QUERY_CONTAINS | Focus::GAME_OVER) ||
                        !focuses.has_focus(Focus::MAIN_MENU))
                    {
                        focuses.pop_back();
                    }
                }
            } else if (focuses.has_focus(Focus::LOADING | Focus::SCENE)) {
                focuses.force_push_back(Focus::MAIN_MENU);
            } else if (!focuses.empty()) {
                throw std::runtime_error("Unknown focus value: " + (std::stringstream() << focuses).str());
            }
        }
    }
}
