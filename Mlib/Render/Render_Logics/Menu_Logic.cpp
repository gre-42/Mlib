#ifdef __ANDROID__
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include "Menu_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Log.hpp>

namespace Mlib {
struct MenuLogicKeys {
    BaseKeyCombination esc{{{.key = "ESCAPE"}}};
    BaseKeyCombination start{{{.key = "ESCAPE", .gamepad_button = "START", .tap_button="ESCAPE"}}};
    BaseKeyCombination F11{{{.key = "F11"}}};
};
}

using namespace Mlib;

MenuLogic::MenuLogic(
#ifndef __ANDROID__
    GLFWwindow &window,
#endif
    MenuUserClass &user_object)
    : user_object_{user_object}
    , button_press_{user_object.button_states}
#ifndef __ANDROID__
    , window_{window}
#endif
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
#ifndef __ANDROID__
    if (button_press_.keys_pressed(keys_->F11)) {
        toggle_fullscreen(window_, user_object_.window_position);
    }
#endif
#ifndef __ANDROID__
    if (user_object_.exit_on_escape) {
        if (button_press_.keys_pressed(keys_->esc)) {
            GLFW_CHK(glfwSetWindowShouldClose(&window_, GLFW_TRUE));
        }
    }
#endif
}

void MenuLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    handle_events();
}

void MenuLogic::print(std::ostream &ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FlyingCameraLogic\n";
}
