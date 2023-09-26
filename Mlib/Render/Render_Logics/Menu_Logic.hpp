#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

struct GLFWwindow;

namespace Mlib {

struct WindowPosition;
class ButtonStates;
class CursorStates;
class Focuses;
struct MenuLogicKeys;

class MenuUserClass {
public:
#ifndef __ANDROID__
    WindowPosition window_position;
#endif
    ButtonStates &button_states;
    CursorStates &cursor_states;
    CursorStates &scroll_wheel_states;
    Focuses &focuses;
    bool exit_on_escape;
};

class MenuLogic: public RenderLogic {
public:
    explicit MenuLogic(
#ifndef __ANDROID__
        GLFWwindow& window,
#endif
        MenuUserClass &user_object);
    ~MenuLogic();

    void handle_events();

    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    MenuUserClass &user_object_;
    ButtonPress button_press_;
#ifndef __ANDROID__
    GLFWwindow& window_;
#endif
    std::unique_ptr<MenuLogicKeys> keys_;
};

}
