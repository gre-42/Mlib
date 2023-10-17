#ifndef __ANDROID__

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Window_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Log.hpp>

namespace Mlib {
struct WindowLogicKeys {
    BaseKeyCombination esc{{{.key = "ESCAPE"}}};
    BaseKeyCombination F11{{{.key = "F11"}}};
};
}

using namespace Mlib;

WindowLogic::WindowLogic(
    GLFWwindow &window,
    WindowUserClass &user_object)
    : user_object_{user_object}
    , button_press_{user_object.button_states}
    , window_{window}
    , keys_{std::make_unique<WindowLogicKeys>()}
{}

WindowLogic::~WindowLogic() = default;

void WindowLogic::handle_events() {
    if (button_press_.keys_pressed(keys_->F11)) {
        toggle_fullscreen(window_, user_object_.window_position);
    }
    if (user_object_.exit_on_escape) {
        if (button_press_.keys_pressed(keys_->esc)) {
            GLFW_CHK(glfwSetWindowShouldClose(&window_, GLFW_TRUE));
        }
    }
}

#endif
