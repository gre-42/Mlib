#include "Renderer.hpp"
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Toggle_Benchmark_Rendering.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Threads/Future_Guard.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Lag_Finder.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <Mlib/Time/Sleep.hpp>
#include <future>

#ifndef __ANDROID__

using namespace Mlib;

Renderer::Renderer(
    Window& window,
    const RenderConfig& render_config,
    const InputConfig& input_config,
    std::atomic_size_t& num_renderings,
    SetFps& set_fps,
    std::function<std::chrono::steady_clock::time_point()> frame_time,
    RenderResults* render_results)
    : window_{ window }
    , render_config_{ render_config }
    , input_config_{ input_config }
    , num_renderings_{ num_renderings }
    , render_results_{ render_results }
    , set_fps_{ set_fps }
    , frame_time_{ std::move(frame_time) }
{
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        THROW_OR_ABORT("Renderer created despite unhandled exception");
    }
    if (!frame_time_) {
        THROW_OR_ABORT("frame_time not set");
    }
}

Renderer::~Renderer() = default;

void Renderer::render(RenderLogic& logic, const SceneGraphConfig& scene_graph_config) const
{
    try {
        GlContextGuard gcg{ window_ };
        RenderTimeId time_id = 0;
        // PeriodicLagFinder lag_finder{ "Render: ", std::chrono::milliseconds{ 100 }};
        set_fps_.tick(std::chrono::steady_clock::time_point());
        while (continue_rendering())
        {
            // lag_finder.start();
            TIME_GUARD_INITIALIZE(1000 * 60, MaxLogLengthExceededBehavior::THROW_EXCEPTION);
            if (num_renderings_ != SIZE_MAX) {
                --num_renderings_;
            }
            int width, height;

            GLFW_CHK(glfwGetFramebufferSize(&window_.glfw_window(), &width, &height));

            // Check if window is minimized.
            if ((width == 0) && (height == 0)) {
                Mlib::sleep_for(std::chrono::milliseconds{ 100 });
                continue;
            }
            if ((width == 0) || (height == 0)) {
                THROW_OR_ABORT("Renderer::operator () received zero width or height");
            }

            ViewportGuard vg{ width, height };

            auto frame_time = frame_time_();
            {
                auto dpi = window_.dpi();
                // TimeGuard time_guard("logic.render", "logic.render");
                RenderedSceneDescriptor rsd{ .external_render_pass = {UINT32_MAX, ExternalRenderPassType::STANDARD, frame_time}, .time_id = time_id };
                // lerr() << "-------------------------------";
                // logic.print(lraw(), 0);
                // lerr() << "+++++++++++++++++++++++++++++++";
                logic.render_toplevel(
                    LayoutConstraintParameters{
                        .dpi = dpi(0),
                        .min_pixel = 0.f,
                        .end_pixel = (float)width},
                    LayoutConstraintParameters{
                        .dpi = dpi(1),
                        .min_pixel = 0.f,
                        .end_pixel = (float)height},
                    render_config_,
                    scene_graph_config,
                    render_results_,
                    rsd);
            }

            if ((render_results_ != nullptr) && (render_results_->output != nullptr)) {
                VectorialPixels<float, 3> vp{ ArrayShape{size_t(height), size_t(width)} };
                CHK(glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, vp->flat_iterable().begin()));
                window_.request_close();
                *render_results_->output = reverted_axis(vp.to_array(), 1);
            }
            if ((render_results_ != nullptr) && !render_results_->outputs.empty()) {
                window_.request_close();
            }
            {
                // Set FPS, assuming that "window_->draw();" below will take 0 time.
                TIME_GUARD_DECLARE(time_guard, "set_fps", "set_fps");
                set_fps_.tick(frame_time);
            }
            {
                TIME_GUARD_DECLARE(time_guard, "window_.draw", "window_.draw");
                window_.draw();
            }
            {
                TIME_GUARD_DECLARE(time_guard, "execute_render_gc", "execute_render_gc");
                execute_render_gc();
            }
            time_id = (time_id + 1) % RENDER_TIME_ID_END;
            #ifdef BENCHMARK_RENDERING_ENABLED
            static size_t ii = 0;
            if (ii++ % 600 == 0) {
                TimeGuard::print_groups(lraw());
            }
            #endif
            // lag_finder.stop();
        }
    } catch (...) {
        window_.request_close();
        throw;
    }
}

class RendererUserClass {
public:
    std::function<void(uint32_t)>* char_callback;
    ButtonStates* button_states;
    CursorStates* cursor_states;
    CursorStates* scroll_wheel_states;
};

void character_callback(GLFWwindow* window, uint32_t codepoint) {
    GLFW_CHK(auto* user_object = (RendererUserClass*)glfwGetWindowUserPointer(window));
    try {
        (*user_object->char_callback)(codepoint);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GLFW_CHK(auto* user_object = (RendererUserClass*)glfwGetWindowUserPointer(window));
    try {
        user_object->button_states->notify_key_event(key, action);
        if ((key == GLFW_KEY_BACKSPACE) &&
            ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) &&
            (user_object->char_callback != nullptr))
        {
            (*user_object->char_callback)('\b');
        }
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
}

static void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    GLFW_CHK(auto* user_object = (RendererUserClass*)glfwGetWindowUserPointer(window));
    try {
        user_object->cursor_states->update_cursor(xpos, ypos);
        GLFW_CHK(glfwSetCursorPos(window, 0, 0));
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    try {
        GLFW_CHK(auto* user_object = (RendererUserClass*)glfwGetWindowUserPointer(window));
        user_object->button_states->notify_mouse_button_event(button, action);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
}

static void scroll_wheel_callback(GLFWwindow* window, double xoffset, double yoffset) {
    try {
        GLFW_CHK(auto* user_object = (RendererUserClass*)glfwGetWindowUserPointer(window));
        user_object->scroll_wheel_states->update_cursor(xoffset, yoffset);
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
}

void Renderer::render_and_handle_events(
    RenderLogic& logic,
    const std::function<void()>& event_handler,
    const SceneGraphConfig& scene_graph_config,
    std::function<void(uint32_t)>* char_callback,
    ButtonStates* button_states,
    CursorStates* cursor_states,
    CursorStates* scroll_wheel_states)
{
    FutureGuard future_guard{
        std::async(std::launch::async, [&](){
            ThreadInitializer ti{"Render", ThreadAffinity::POOL};
            render(logic, scene_graph_config);
        })};
    handle_events(*this, char_callback, button_states, cursor_states, scroll_wheel_states, input_config_, event_handler);
}

bool Renderer::continue_rendering() const {
    // Mlib::sleep_for(std::chrono::milliseconds(500));
    // From: https://www.glfw.org/docs/latest/context_guide.html#context_current
    return
        !window_.close_requested() &&
        (num_renderings_ != 0) &&
        !unhandled_exceptions_occured();
}

void Mlib::handle_events(
    Renderer& renderer,
    std::function<void(uint32_t)>* char_callback,
    ButtonStates* button_states,
    CursorStates* cursor_states,
    CursorStates* scroll_wheel_states,
    const InputConfig& input_config,
    const std::function<void()>& callback)
{
    RendererUserClass user_object{
        .char_callback = char_callback,
        .button_states = button_states,
        .cursor_states = cursor_states,
        .scroll_wheel_states = scroll_wheel_states};
    try {
        DestructionGuards dgs;
        GLFWwindow* window_handle = &renderer.window_.glfw_window();
        GLFW_CHK(glfwSetWindowUserPointer(window_handle, &user_object));
        dgs.add([window_handle]() {GLFW_ABORT(glfwSetWindowUserPointer(window_handle, nullptr));});
        if (char_callback != nullptr) {
            GLFW_CHK(glfwSetCharCallback(window_handle, character_callback));
            dgs.add([window_handle]() {GLFW_ABORT(glfwSetCharCallback(window_handle, nullptr));});
        }
        if (button_states != nullptr) {
            GLFW_CHK(glfwSetKeyCallback(window_handle, key_callback));
            dgs.add([window_handle]() {GLFW_ABORT(glfwSetKeyCallback(window_handle, nullptr));});
            GLFW_CHK(glfwSetMouseButtonCallback(window_handle, mouse_button_callback));
            dgs.add([window_handle]() {GLFW_ABORT(glfwSetMouseButtonCallback(window_handle, nullptr));});
        }
        if (cursor_states != nullptr) {
            GLFW_CHK(glfwSetCursorPosCallback(window_handle, cursor_callback));
            dgs.add([window_handle]() {GLFW_ABORT(glfwSetCursorPosCallback(window_handle, nullptr));});
        }
        if (scroll_wheel_states != nullptr) {
            GLFW_CHK(glfwSetScrollCallback(window_handle, scroll_wheel_callback));
            dgs.add([window_handle]() {GLFW_ABORT(glfwSetScrollCallback(window_handle, nullptr));});
        }
        // PeriodicLagFinder lag_finder{ "Events: ", std::chrono::milliseconds{ 100 }};
        while (renderer.continue_rendering()) {
            // lag_finder.start();
            // GLFW_CHK(glfwPollEvents());
            GLFW_CHK(glfwWaitEventsTimeout(input_config.polling_interval_seconds));
            if (button_states != nullptr) {
                button_states->update_gamepad_state();
            }
            // lag_finder.stop();
            if (callback) {
                callback();
            }
        }
    } catch (...) {
        renderer.window_.request_close();
        throw;
    }
}

#endif
