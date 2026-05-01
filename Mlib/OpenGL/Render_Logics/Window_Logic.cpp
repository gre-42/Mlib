#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Window_Logic.hpp"
#include <Mlib/Iterator/Span.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/OpenGL/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/OpenGL/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/OpenGL/Ui/Button_States.hpp>
#include <stdexcept>
#include <Mlib/Misc/Log.hpp>

using S = Mlib::VariableAndHash<std::string>;

namespace Mlib {
class WindowLogicKeys {
public:
    explicit WindowLogicKeys(ButtonStates& button_states)
        : esc{ button_states, key_configurations, 0, "esc", "" }
        , F11{ button_states, key_configurations, 0, "F11", "" }
    {
        auto lock = key_configurations.lock_exclusive_for(std::chrono::seconds(2), "Key configurations");
        lock->insert(0, "esc", { {{{.key = S("ESCAPE")}}} });
        lock->insert(0, "F11", { {{{.key = S("F11")}}} });
    }
    ButtonPress esc;
    ButtonPress F11;
private:
    LockableKeyConfigurations key_configurations;
};
}

using namespace Mlib;

std::string VideoMode::to_string() const {
    return (std::stringstream() << "  " << width << " x " << height << " @ " << refresh_rate << " Hz").str();
}

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
    {
        std::scoped_lock lock{ mutex_ };
        if (!fullscreen_modes_.has_value()) {
            auto* monitor = get_primary_monitor();
            if (monitor == nullptr) {
                lwarn() << "Could not obtain primary monitor";
            } else {
                int mode_count;
                const GLFWvidmode* modes = GLFW_CHK_X(glfwGetVideoModes(monitor, &mode_count));
                fullscreen_modes_.emplace();
                fullscreen_modes_->reserve(integral_cast<size_t>(mode_count));
                for (const auto& mode : Span(modes, integral_cast<size_t>(mode_count))) {
                    fullscreen_modes_->emplace_back(
                        mode.width,
                        mode.height,
                        mode.redBits,
                        mode.greenBits,
                        mode.blueBits,
                        mode.refreshRate);
                    // linfo() << "  " << mode.width << " x " << mode.height << " @ " << mode.refreshRate << " Hz";
                }
            }
        }
    }
    {
        std::shared_lock lock{ mutex_ };
        if (desired_mode_.has_value()) {
            if (!is_fullscreen()) {
                // Backup window position and size before going to fullscreen.
                GLFW_CHK(glfwGetWindowPos(&window_, &user_object_.window_position.windowed_x, &user_object_.window_position.windowed_y));
                GLFW_CHK(glfwGetWindowSize(&window_, &user_object_.window_position.windowed_width, &user_object_.window_position.windowed_height));
            }
            auto* monitor = get_primary_monitor();
            if (monitor == nullptr) {
                lwarn() << "Could not obtain primary monitor";
            } else {
                const char* monitor_name = GLFW_CHK_X(glfwGetMonitorName(monitor));
                if (monitor_name == nullptr) {
                    lwarn() << "Could not obtain monitor name";
                } else {
                    linfo() <<
                        "Going to full screen (monitor: \"" << monitor_name << "\", width: " << desired_mode_->width <<
                        ", height: " << desired_mode_->height << ", refresh rate: " << desired_mode_->refresh_rate << " Hz)";
                    GLFW_CHK(glfwSetWindowMonitor(
                        &window_,
                        monitor,
                        0,
                        0,
                        desired_mode_->width,
                        desired_mode_->height,
                        desired_mode_->refresh_rate));
                    // GLFW_CHK(glfwSetWindowSize(
                    //     &window_,
                    //     desired_mode_->width,
                    //     desired_mode_->height));
                }
            }
            desired_mode_.reset();
        }
    }
}

bool WindowLogic::is_fullscreen() const {
    GLFWmonitor* window_monitor = GLFW_CHK_X(glfwGetWindowMonitor(&window_));
    return (window_monitor != nullptr);
}

GLFWmonitor* WindowLogic::get_primary_monitor() const {
    GLFWmonitor* monitor = GLFW_CHK_X(glfwGetPrimaryMonitor());
    return monitor;
}

void WindowLogic::clear_fullscreen_modes() {
    std::scoped_lock lock{ mutex_ };
    fullscreen_modes_.reset();
}

std::vector<VideoMode> WindowLogic::fullscreen_modes() const {
    std::shared_lock lock{ mutex_ };
    if (fullscreen_modes_.has_value()) {
        return *fullscreen_modes_;
    } else {
        return {};
    }
}

void WindowLogic::set_fullscreen_mode(const DesiredVideoMode& mode) {
    std::scoped_lock lock{ mutex_ };
    desired_mode_ = mode;
}

#else

#include "Window_Logic.hpp"
#include <stdexcept>
#include <sstream>

using namespace Mlib;

std::string VideoMode::to_string() const {
    return (std::stringstream() << "  " << width << " x " << height << " @ " << refresh_rate << " Hz").str();
}

WindowLogic::WindowLogic() = default;
WindowLogic::~WindowLogic() = default;

void WindowLogic::handle_events() {
    throw std::runtime_error("WindowLogic::handle_events not implemented");
}

void WindowLogic::clear_fullscreen_modes() {
    throw std::runtime_error("WindowLogic::clear_fullscreen_modes not implemented");
}

std::vector<VideoMode> WindowLogic::fullscreen_modes() const {
    throw std::runtime_error("WindowLogic::fullscreen_modes not implemented");
}

void WindowLogic::set_fullscreen_mode(const DesiredVideoMode& mode) {
    throw std::runtime_error("WindowLogic::set_fullscreen_mode not implemented");
}

bool WindowLogic::is_fullscreen() const {
    throw std::runtime_error("WindowLogic::is_fullscreen not implemented");
}

#endif
