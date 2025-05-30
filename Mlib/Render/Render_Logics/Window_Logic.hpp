#ifndef __ANDROID__

#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <optional>
#include <vector>

struct GLFWmonitor;
struct GLFWwindow;

namespace Mlib {

struct WindowPosition;
class ButtonStates;
class CursorStates;
class Focuses;
class WindowLogicKeys;

class WindowUserClass {
public:
    WindowPosition window_position;
    ButtonStates &button_states;
    bool exit_on_escape;
};

struct VideoMode {
    int width;
    int height;
    int red_bits;
    int green_bits;
    int blue_bits;
    int refresh_rate;
    std::string to_string() const;
};

struct DesiredVideoMode {
    int width;
    int height;
    int refresh_rate;
};

class WindowLogic {
    WindowLogic(const WindowLogic&) = delete;
    WindowLogic& operator = (const WindowLogic&) = delete;
public:
    WindowLogic(
        GLFWwindow& window,
        WindowUserClass &user_object);
    ~WindowLogic();

    void handle_events();
    void clear_fullscreen_modes();
    std::vector<VideoMode> fullscreen_modes() const;
    void set_fullscreen_mode(const DesiredVideoMode& mode);
    bool is_fullscreen() const;

private:
    GLFWmonitor* get_primary_monitor() const;
    
    WindowUserClass &user_object_;
    GLFWwindow& window_;
    std::unique_ptr<WindowLogicKeys> keys_;
    mutable SafeAtomicSharedMutex mutex_;
    std::optional<std::vector<VideoMode>> fullscreen_modes_;
    std::optional<DesiredVideoMode> desired_mode_;
};

}

#endif
