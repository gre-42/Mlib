#include "Renderer.hpp"
#include <Mlib/Fps/Fps.hpp>
#include <Mlib/Fps/Lag_Finder.hpp>
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Toggle_Benchmark_Rendering.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Threads/Future_Guard.hpp>
#include <Mlib/Threads/Set_Thread_Name.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <future>

using namespace Mlib;

Renderer::Renderer(
    Window& window,
    const RenderConfig& render_config,
    size_t& num_renderings,
    RenderResults* render_results)
: window_{window},
  render_config_{render_config},
  num_renderings_{num_renderings},
  render_results_{render_results}
{
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        throw std::runtime_error("Renderer created despite unhandled exception");
    }
}

Renderer::~Renderer()
{}

void Renderer::render(RenderLogic& logic, const SceneGraphConfig& scene_graph_config) const
{
    try {
        set_thread_name("Renderer");
        GlContextGuard gcg{ window_.window() };
        SetFps set_fps{"Render FPS: "};
        Fps fps;
        size_t fps_i = 0;
        size_t fps_i_max = 500;
        size_t time_id = 0;
        // LagFinder lag_finder{ "Render: ", std::chrono::milliseconds{ 100 }};
        while (continue_rendering())
        {
            // lag_finder.start();
            TIME_GUARD_INITIALIZE(1000 * 60, MaxLogLengthExceededBehavior::THROW_EXCEPTION);
            if (num_renderings_ != SIZE_MAX) {
                --num_renderings_;
            }
            int width, height;

            GLFW_CHK(glfwGetFramebufferSize(window_.window(), &width, &height));

            // Check if window is minimized.
            if ((width == 0) && (height == 0)) {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
                continue;
            }
            if ((width == 0) || (height == 0)) {
                throw std::runtime_error("Renderer::operator () received zero width or height");
            }

            ViewportGuard vg{ 0, 0, width, height };

            {
                // TimeGuard time_guard("logic.render", "logic.render");
                RenderedSceneDescriptor rsd{ .external_render_pass = {ExternalRenderPassType::STANDARD, ""}, .time_id = time_id };
                // std::cerr << "-------------------------------" << std::endl;
                // logic.print(std::cerr, 0);
                // std::cerr << "+++++++++++++++++++++++++++++++" << std::endl;
                logic.render(
                    width,
                    height,
                    render_config_,
                    scene_graph_config,
                    render_results_,
                    rsd);
            }

            if (render_results_ != nullptr && render_results_->output != nullptr) {
                VectorialPixels<float, 3> vp{ ArrayShape{size_t(height), size_t(width)} };
                CHK(glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, vp->flat_iterable().begin()));
                GLFW_CHK(glfwSetWindowShouldClose(window_.window(), GLFW_TRUE));
                *render_results_->output = reverted_axis(vp.to_array(), 1);
            }
            if (render_results_ != nullptr && !render_results_->outputs.empty()) {
                GLFW_CHK(glfwSetWindowShouldClose(window_.window(), GLFW_TRUE));
            } else if (render_config_.dt != 0) {
                // Set FPS, assuming that "window_->draw();" below will take 0 time.
                TIME_GUARD_DECLARE(time_guard, "set_fps", "set_fps");
                set_fps.tick(
                    render_config_.dt,
                    render_config_.max_residual_time,
                    render_config_.control_fps,
                    render_config_.print_residual_time);
            } else if (render_config_.motion_interpolation) {
                throw std::runtime_error("Motion interpolation requires render_dt");
            }
            {
                TIME_GUARD_DECLARE(time_guard, "window_->draw", "window_->draw");
                window_.draw();
            }
            // Compute FPS, including the time that "window_->draw();" took.
            if (render_config_.print_fps) {
                fps_i = (fps_i + 1) % fps_i_max;
                fps.tick();
                if (fps_i == 0) {
                    std::cerr << "Render FPS: Mean = " << fps.mean_fps() << ", MAD = " << fps.mad_fps() << std::endl;
                }
                // if (fps.last_fps() < 55) {
                //     std::cerr << "FPS < 55" << std::endl;
                // }
            }
            {
                TIME_GUARD_DECLARE(time_guard, "execute_gc_render", "execute_gc_render");
                execute_gc_render();
            }
            if (render_config_.motion_interpolation) {
                time_id = (time_id + 1) % 4;
            }
            #ifdef BENCHMARK_RENDERING_ENABLED
            static size_t ii = 0;
            if (ii++ % 600 == 0) {
                TimeGuard::print_groups(std::cerr);
            }
            #endif
            // lag_finder.stop();
        }
    } catch (const std::runtime_error&) {
        GLFW_CHK(glfwSetWindowShouldClose(window_.window(), GLFW_TRUE));
        throw;
    }
}

class RendererUserClass {
public:
    ButtonStates* button_states;
    CursorStates* cursor_states;
    CursorStates* scroll_wheel_states;
};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GLFW_CHK(auto* user_object = (RendererUserClass*)glfwGetWindowUserPointer(window));
    try {
        user_object->button_states->notify_key_event(key, action);
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
    const SceneGraphConfig& scene_graph_config,
    ButtonStates* button_states,
    CursorStates* cursor_states,
    CursorStates* scroll_wheel_states)
{
    FutureGuard future_guard{
        std::async(std::launch::async, [&](){
            render(logic, scene_graph_config);
        })};
    EventHandler(*this, button_states, cursor_states, scroll_wheel_states);
}

bool Renderer::continue_rendering() const {
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // From: https://www.glfw.org/docs/latest/context_guide.html#context_current
    return
        !glfwWindowShouldClose(window_.window()) &&
        (num_renderings_ != 0) &&
        !unhandled_exceptions_occured();
}

EventHandler::EventHandler(
    Renderer& renderer,
    ButtonStates* button_states,
    CursorStates* cursor_states,
    CursorStates* scroll_wheel_states)
: renderer_{renderer},
  button_states_{button_states},
  cursor_states_{cursor_states},
  scroll_wheel_states_{scroll_wheel_states}
{
    RendererUserClass user_object{
        .button_states = button_states,
        .cursor_states = cursor_states,
        .scroll_wheel_states = scroll_wheel_states};
    try {
        GLFW_CHK(glfwSetWindowUserPointer(renderer_.window_.window(), &user_object));
        if (button_states != nullptr) {
            GLFW_CHK(glfwSetKeyCallback(renderer_.window_.window(), key_callback));
            GLFW_CHK(glfwSetMouseButtonCallback(renderer_.window_.window(), mouse_button_callback));
        }
        if (cursor_states != nullptr) {
            GLFW_CHK(glfwSetCursorPosCallback(renderer_.window_.window(), cursor_callback));
        }
        if (scroll_wheel_states != nullptr) {
            GLFW_CHK(glfwSetScrollCallback(renderer_.window_.window(), scroll_wheel_callback));
        }
        // LagFinder lag_finder{ "Events: ", std::chrono::milliseconds{ 100 }};
        while (renderer_.continue_rendering()) {
            // lag_finder.start();
            GLFW_CHK(glfwPollEvents());
            if (button_states != nullptr) {
                button_states->update_gamepad_state();
            }
            // lag_finder.stop();
        }
    } catch (const std::runtime_error& e) {
        GLFW_WARN(glfwSetWindowShouldClose(renderer_.window_.window(), GLFW_TRUE));
        throw;
    }
}

EventHandler::~EventHandler() {
    GLFW_WARN(glfwSetWindowUserPointer(renderer_.window_.window(), nullptr));
    if (button_states_ != nullptr) {
        GLFW_WARN(glfwSetKeyCallback(renderer_.window_.window(), nullptr));
        GLFW_WARN(glfwSetMouseButtonCallback(renderer_.window_.window(), nullptr));
    }
    if (cursor_states_ != nullptr) {
        GLFW_WARN(glfwSetCursorPosCallback(renderer_.window_.window(), nullptr));
    }
    if (scroll_wheel_states_ != nullptr) {
        GLFW_WARN(glfwSetScrollCallback(renderer_.window_.window(), nullptr));
    }
}
