#ifndef __ANDROID__

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Window_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Log.hpp>

namespace Mlib {
class WindowLogicKeys {
public:
    explicit WindowLogicKeys(ButtonStates& button_states)
        : esc{ button_states, key_configurations, 0, "esc", "" }
        , F11{ button_states, key_configurations, 0, "F11", "" }
    {
        auto lock = key_configurations.lock_exclusive_for(std::chrono::seconds(2), "Key configurations");
        lock->insert(0, "esc", { {{{.key = "ESCAPE"}}} });
        lock->insert(0, "F11", { {{{.key = "F11"}}} });
    }
    ButtonPress esc;
    ButtonPress F11;
private:
    LockableKeyConfigurations key_configurations;
};
}

using namespace Mlib;

WindowLogic::WindowLogic(
    GLFWwindow &window,
    WindowUserClass &user_object)
    : user_object_{ user_object }
    , window_{ window }
    , keys_{ std::make_unique<WindowLogicKeys>(user_object.button_states) }
{}

WindowLogic::~WindowLogic() = default;

void WindowLogic::handle_events() {
    if (keys_->F11.keys_pressed()) {
        toggle_fullscreen(window_, user_object_.window_position);
    }
    if (user_object_.exit_on_escape) {
        if (keys_->esc.keys_pressed()) {
            GLFW_CHK(glfwSetWindowShouldClose(&window_, GLFW_TRUE));
        }
    }
}

#endif
