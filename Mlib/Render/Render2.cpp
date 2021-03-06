#include "Render2.hpp"
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Fps/Fps.hpp>
#include <Mlib/Fps/Lag_Finder.hpp>
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Rotating_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Toggle_Benchmark_Rendering.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Threads/Set_Thread_Name.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <thread>

using namespace Mlib;

static void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

/*static void print_mat4x4(const mat4x4& m) {
    for (size_t r = 0; r < 4; ++r) {
        for (size_t c = 0; c < 4; ++c) {
            std::cerr << m[r][c] << " ";
        }
        std::cerr << std::endl;
    }
}*/

Render2::Render2(
    const RenderConfig& render_config,
    size_t& num_renderings,
    RenderResults* render_results)
: num_renderings_{num_renderings},
  render_results_{render_results},
  render_config_{render_config}
{
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("glfwInit failed");
    }
    GLFW_CHK(glfwSetErrorCallback(error_callback));

    GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, render_config.opengl_major_version));
    GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, render_config.opengl_minor_version));
    if (render_results != nullptr && (render_results->output != nullptr || !render_results->outputs.empty())) {
        GLFW_CHK(glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE));
    }
    if (render_config.nsamples_msaa != 1) {
        GLFW_CHK(glfwWindowHint(GLFW_SAMPLES, render_config.nsamples_msaa));
    }
    if (render_config.fullscreen) {
        GLFW_CHK(glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE));
    }
    {
#ifdef __linux__
        TemporarilyIgnoreFloatingPointExeptions ignore_except;
#endif
        GLFWmonitor* monitor = !render_config.fullscreen
            ? nullptr
            : GLFW_CHK(glfwGetPrimaryMonitor());
        window_ = std::make_unique<Window>(
            render_config.fullscreen ? render_config.fullscreen_width : render_config.windowed_width,
            render_config.fullscreen ? render_config.fullscreen_height : render_config.windowed_height,
            render_config.window_title.c_str(),
            monitor,
            nullptr,
            render_config.double_buffer,
            render_config.swap_interval);
    }
    if (!render_config.show_mouse_cursor) {
        GLFW_CHK(glfwSetInputMode(window_->window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED));
    }
    {
        GlContextGuard gcg{window_->window()};
        CHK(int version = gladLoadGL(glfwGetProcAddress));
        if (version == 0) {
            throw std::runtime_error("gladLoadGL failed");
        }
    }
}

Render2::~Render2() {
    {
        // This internally calls "execute_gc_render"
        GlContextGuard gcg{ window_->window() };
    }
    window_.release();
    GLFW_WARN(glfwTerminate());
}

void Render2::print_hardware_info() const {
    GlContextGuard gcg{ window_->window() };
    const char* vendor = CHK((const char*)glGetString(GL_VENDOR));
    const char* renderer = CHK((const char*)glGetString(GL_RENDERER));
    std::cerr << "Vendor: " << vendor << std::endl;
    std::cerr << "Renderer: " << renderer << std::endl;
}

void Render2::operator () (
    RenderLogic& logic,
    const SceneGraphConfig& scene_graph_config,
    ButtonStates* button_states)
{
    if (unhandled_exceptions_occured()) {
        print_unhandled_exceptions();
        throw std::runtime_error("Render2 called despite unhandled exception");
    }
    SetFps set_fps{"Render FPS: "};
    Fps fps;
    size_t fps_i = 0;
    size_t fps_i_max = 500;
    size_t time_id = 0;
    // Get current keyboard inputs in case the scene was reloaded.
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    GLFW_CHK(glfwPollEvents());
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // From: https://www.glfw.org/docs/latest/context_guide.html#context_current
    auto continue_rendering = [&]() { return
            !glfwWindowShouldClose(window_->window()) &&
            (num_renderings_ != 0) &&
            !unhandled_exceptions_occured(); };
    std::exception_ptr teptr = nullptr;
    auto render_thread_func = [&]() {
        try {
            set_thread_name("Render2");
            GlContextGuard gcg{ window_->window() };
            // LagFinder lag_finder{ "Render: ", std::chrono::milliseconds{ 100 }};
            while (continue_rendering())
            {
                // lag_finder.start();
                TIME_GUARD_INITIALIZE(1000 * 60, MaxLogLengthExceededBehavior::THROW_EXCEPTION);
                if (num_renderings_ != SIZE_MAX) {
                    --num_renderings_;
                }
                int width, height;

                GLFW_CHK(glfwGetFramebufferSize(window_->window(), &width, &height));

                // Check if window is minimized.
                if ((width == 0) && (height == 0)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
                    continue;
                }
                if ((width == 0) || (height == 0)) {
                    throw std::runtime_error("Render2::operator () received zero width or height");
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
                    GLFW_CHK(glfwSetWindowShouldClose(window_->window(), GLFW_TRUE));
                    *render_results_->output = reverted_axis(vp.to_array(), 1);
                }
                if (render_results_ != nullptr && !render_results_->outputs.empty()) {
                    GLFW_CHK(glfwSetWindowShouldClose(window_->window(), GLFW_TRUE));
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
                    window_->draw();
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
            GLFW_CHK(glfwSetWindowShouldClose(window_->window(), GLFW_TRUE));
            teptr = std::current_exception();
        }
    };
    auto thread_runner = RenderingContextStack::generate_thread_runner(
        RenderingContextStack::primary_resource_context(),
        RenderingContextStack::resource_context());
    std::thread render_thread{ thread_runner(render_thread_func) };
    // LagFinder lag_finder{ "Events: ", std::chrono::milliseconds{ 100 }};
    while (continue_rendering()) {
        // lag_finder.start();
        GLFW_CHK(glfwPollEvents());
        if (button_states != nullptr) {
            button_states->update_gamepad_state();
        }
        // lag_finder.stop();
    }
    render_thread.join();
    if (teptr != nullptr) {
        std::rethrow_exception(teptr);
    }
}

void Render2::operator () (
    const Scene& scene,
    const FixedArray<float, 3>& background_color,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations)
{
    RotatingLogic rotating_logic{
        window_->window(),
        scene,
        rotate,
        scale,
        camera_z,
        background_color,
        beacon_locations};
    ReadPixelsLogic read_pixels_logic{ rotating_logic };
    (*this)(
        read_pixels_logic,
        scene_graph_config);
}

void Render2::render_node(
    std::unique_ptr<SceneNode>&& node,
    const FixedArray<float, 3>& background_color,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    std::unique_ptr<Camera>&& camera,
    const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations)
{
    DeleteNodeMutex delete_node_mutex;
    Scene scene{ delete_node_mutex };
    scene.add_root_node("obj", std::move(node));
    scene.add_root_node("camera", std::make_unique<SceneNode>());
    // std::make_shared<GenericCamera>(camera_config, GenericCamera::Mode::PERSPECTIVE)
    scene.get_node("camera").set_camera(std::move(camera));
    scene.add_root_node("light", std::make_unique<SceneNode>());
    scene.get_node("light").add_light(std::make_unique<Light>(Light{
        .ambience = {0.5f, 0.5f, 0.5f},
        .diffusivity = {1.f, 1.f, 1.f},
        .specularity = {1.f, 1.f, 1.f},
        .shadow_render_pass = ExternalRenderPassType::NONE}));
    (*this)(scene, background_color, rotate, scale, camera_z, scene_graph_config, beacon_locations);
}

GLFWwindow* Render2::window() const {
    assert_true(window_.get());
    return window_->window();
}

bool Render2::window_should_close() const {
    return (window_ != nullptr) && GLFW_CHK(glfwWindowShouldClose(window_->window()));
}
