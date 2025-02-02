#ifndef __ANDROID__

#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

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

class WindowLogic {
public:
    WindowLogic(
        GLFWwindow& window,
        WindowUserClass &user_object);
    ~WindowLogic();

    void handle_events();

private:
    WindowUserClass &user_object_;
    GLFWwindow& window_;
    std::unique_ptr<WindowLogicKeys> keys_;
};

}

#endif
