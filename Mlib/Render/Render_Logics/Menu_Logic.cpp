#include "Menu_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {
struct MenuLogicKeys {
    BaseKeyCombination start{{{.key = "ESCAPE", .gamepad_button = "START", .tap_button="ESCAPE"}}};
};
}

using namespace Mlib;

MenuLogic::MenuLogic(
    MenuUserClass &user_object)
    : user_object_{user_object}
    , button_press_{user_object.button_states}
    , keys_{std::make_unique<MenuLogicKeys>()}
{}

MenuLogic::~MenuLogic() = default;

void MenuLogic::handle_events() {
    LOG_FUNCTION("FlyingCameraLogic::render");
    if (button_press_.keys_pressed(keys_->start)) {
        std::scoped_lock lock{user_object_.focuses.mutex};
        Focus focus = user_object_.focuses.focus();
        if (focus == Focus::MENU) {
            if (user_object_.focuses.size() > 1) {
                user_object_.focuses.pop_back();
            }
        } else if (user_object_.focuses.countdown_active() || any(focus & (Focus::LOADING | Focus::SCENE | Focus::GAME_OVER))) {
            user_object_.focuses.push_back(Focus::MENU);
        } else if (user_object_.focuses.game_over_countdown_active()) {
            // Do nothing, menu will show automatically after the countdown is finished
        } else if (focus != Focus::BASE) {
            THROW_OR_ABORT("Unknown focus value: " + std::to_string((int)focus));
        }
    }
}
