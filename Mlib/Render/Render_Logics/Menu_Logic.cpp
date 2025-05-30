#include "Menu_Logic.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstddef>
#include <sstream>

namespace Mlib {
class MenuLogicKeys {
public:
    explicit MenuLogicKeys(ButtonStates& button_states)
        : start{
            {button_states, key_configurations, 0, "escape", ""},
            {button_states, key_configurations, 1, "escape", ""} }
    {
        key_configurations
            .lock_exclusive_for(std::chrono::seconds(2), "Key configurations")
            ->insert(0, "escape", { {{{.key = "ESCAPE", .gamepad_button = {0, "START"}, .tap_button = {0, "ESCAPE"}}}} });
        key_configurations
            .lock_exclusive_for(std::chrono::seconds(2), "Key configurations")
            ->insert(1, "escape", { {{{.gamepad_button = {1, "START"}, .tap_button = {1, "ESCAPE"}}}} });
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
            auto& focuses = user_object_.ui_focuses[user_id].focuses;
            std::scoped_lock lock{focuses.mutex};
            if (focuses.has_focus(Focus::MENU_ANY)) {
                if (focuses.size() > 1) {
                    focuses.pop_back();
                }
            } else if (focuses.countdown_active() || focuses.has_focus(Focus::LOADING | Focus::SCENE | Focus::GAME_OVER)) {
                focuses.push_back(Focus::MAIN_MENU);
            } else if (focuses.game_over_countdown_active()) {
                // Do nothing, menu will show automatically after the countdown is finished
            } else if (!focuses.empty()) {
                THROW_OR_ABORT("Unknown focus value: " + (std::stringstream() << focuses).str());
            }
        }
    }
}
