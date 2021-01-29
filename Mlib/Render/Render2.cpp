#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Render2.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Fps.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Render_Logics/Rotating_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Renderables/Renderable_Depth_Map.hpp>
#include <Mlib/Render/Renderables/Renderable_Height_Map.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Render/linmath.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Set_Fps.hpp>
#include <Mlib/Time_Guard.hpp>
#include <fenv.h>
#include <iostream>

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
    size_t& num_renderings,
    RenderResults* render_results,
    const RenderConfig& render_config)
: num_renderings_{num_renderings},
  render_results_{render_results},
  render_config_{render_config}
{
    GLFW_WARN(glfwSetErrorCallback(error_callback));

    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }

    GLFW_WARN(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3));
    GLFW_WARN(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3));
    if (render_results != nullptr && (render_results->output != nullptr || !render_results->outputs.empty())) {
        GLFW_WARN(glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE));
    }
    if (render_config.nsamples_msaa != 1) {
        GLFW_WARN(glfwWindowHint(GLFW_SAMPLES, render_config.nsamples_msaa));
    }
    if (render_config.window_maximized) {
        GLFW_WARN(glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE));
    }
#ifndef WIN32
    int fpeflags = fegetexcept();
    fedisableexcept(FE_ALL_EXCEPT);
#endif
    if (window_ != nullptr) {
        throw std::runtime_error("Multiple calls to render2");
    }
    GLFWmonitor* monitor = !render_config.full_screen
        ? nullptr
        : GLFW_CHK(glfwGetPrimaryMonitor());
    window_ = std::make_unique<Window>(
        render_config.screen_width,
        render_config.screen_height,
        render_config.window_title.c_str(),
        monitor,
        nullptr);
#ifndef WIN32
    feenableexcept(fpeflags);
#endif
    if (!render_config.show_mouse_cursor) {
        GLFW_CHK(glfwSetInputMode(window_->window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED));
    }
    GLFW_WARN(glfwMakeContextCurrent(window_->window()));
    CHK(int version = gladLoadGL(glfwGetProcAddress));
    if (version == 0) {
        throw std::runtime_error("gladLoadGL failed");
    }
    GLFW_CHK(glfwSwapInterval(render_config.swap_interval));
}

Render2::~Render2() {
    execute_gc_render();
    window_.release();
    GLFW_WARN(glfwTerminate());
}

void Render2::print_hardware_info() const {
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
    CHK(glfwMakeContextCurrent(nullptr));
    auto continue_rendering = [&]() { return !glfwWindowShouldClose(window_->window()) && (num_renderings_ != 0); };
    auto render_thread_func = [&]() {
        CHK(glfwMakeContextCurrent(window_->window()));
        while (continue_rendering())
        {
            // TimeGuard::initialize(5 * 60);
            if (num_renderings_ != SIZE_MAX) {
                --num_renderings_;
            }
            int width, height;

            GLFW_CHK(glfwGetFramebufferSize(window_->window(), &width, &height));

            ViewportGuard vg{ 0, 0, width, height };

            logic.render(
                width,
                height,
                render_config_,
                scene_graph_config,
                render_results_,
                (render_results_ != nullptr) && (!render_results_->outputs.empty())
                ? RenderedSceneDescriptor{ .external_render_pass = {ExternalRenderPassType::STANDARD_WITH_POSTPROCESSING, ""}, .time_id = time_id, .light_node_name = "" }
            : RenderedSceneDescriptor{ .external_render_pass = {ExternalRenderPassType::UNDEFINED, ""}, .time_id = time_id, .light_node_name = "" });

            if (render_results_ != nullptr && render_results_->output != nullptr) {
                VectorialPixels<float, 3> vp{ ArrayShape{size_t(height), size_t(width)} };
                CHK(glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, vp->flat_iterable().begin()));
                GLFW_CHK(glfwSetWindowShouldClose(window_->window(), GLFW_TRUE));
                *render_results_->output = reverted_axis(vp.to_array(), 1);
            }
            if (render_results_ != nullptr && !render_results_->outputs.empty()) {
                GLFW_CHK(glfwSetWindowShouldClose(window_->window(), GLFW_TRUE));
            }
            if (render_config_.dt != 0) {
                // TimeGuard time_guard("set_fps", "set_fps");
                set_fps.tick(render_config_.dt, render_config_.max_residual_time, render_config_.print_residual_time);
            }
            else if (render_config_.motion_interpolation) {
                throw std::runtime_error("Motion interpolation requires render_dt");
            }
            if (render_config_.print_fps) {
                fps_i = (fps_i + 1) % fps_i_max;
                fps.tick();
                if (fps_i == 0) {
                    std::cerr << "Render FPS: Mean = " << fps.mean_fps() << ", MAD = " << fps.mad_fps() << std::endl;
                }
            }
            {
                // TimeGuard time_guard("glfwSwapBuffers", "glfwSwapBuffers");
                GLFW_CHK(glfwSwapBuffers(window_->window()));
            }
            {
                // TimeGuard time_guard("execute_gc_render", "execute_gc_render");
                execute_gc_render();
            }

            if (render_config_.motion_interpolation) {
                time_id = (time_id + 1) % 4;
            }
            // static size_t ii = 0;
            // if (ii++ % 600 == 0) {
            //     TimeGuard::print_groups(std::cerr);
            // }
        }
        CHK(glfwMakeContextCurrent(nullptr));
    };
    std::thread render_thread{ render_thread_func };
    while (continue_rendering()) {
        GLFW_CHK(glfwPollEvents());
        if (button_states != nullptr) {
            button_states->update_gamepad_state();
        }
    }
    render_thread.join();
    CHK(glfwMakeContextCurrent(window_->window()));
}

class LockingRenderLogic: public RenderLogic {
public:
    explicit LockingRenderLogic(
        RenderLogic& child_logic,
        std::recursive_mutex& mutex)
    : child_logic_{child_logic},
      mutex_{mutex}
    {}
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id)
    {
        std::lock_guard lock{mutex_}; // formerly shared_lock
        child_logic_.render(
            width,
            height,
            render_config,
            scene_graph_config,
            render_results,
            frame_id);
    }
private:
    RenderLogic& child_logic_;
    std::recursive_mutex& mutex_;
};

void Render2::operator () (
    RenderLogic& logic,
    std::recursive_mutex& mutex,
    const SceneGraphConfig& scene_graph_config,
    ButtonStates* button_states)
{
    LockingRenderLogic lrl{logic, mutex};
    (*this)(lrl, scene_graph_config, button_states);
}

void Render2::operator () (
    const Scene& scene,
    bool rotate,
    float scale,
    const SceneGraphConfig& scene_graph_config)
{
    RotatingLogic rotating_logic{window_->window(), scene, rotate, scale};
    std::recursive_mutex mutex;
    (*this)(
        rotating_logic,
        mutex,
        scene_graph_config);
}

void Render2::render_depth_map(
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const FixedArray<float, 3, 3>& intrinsic_matrix,
    bool rotate,
    float scale,
    const SceneGraphConfig& scene_graph_config,
    const CameraConfig& camera_config)
{
    const auto r = std::make_shared<RenderableDepthMap>(rgb_picture, depth_picture, intrinsic_matrix);
    SceneNodeResources scene_node_resources;
    Scene scene;
    scene_node_resources.add_resource("RenderableDepthMap", r);
    auto on = new SceneNode;
    scene_node_resources.instantiate_renderable("RenderableDepthMap", "RenderableDepthMap", *on, SceneNodeResourceFilter());
    scene.add_root_node("obj", on);
    scene.add_root_node("camera", new SceneNode);
    scene.get_node("camera")->set_camera(std::make_shared<GenericCamera>(camera_config, GenericCamera::Mode::PERSPECTIVE));
    scene.add_root_node("light", new SceneNode);
    scene.get_node("light")->add_light(new Light{
        .ambience = {0.5f, 0.5f, 0.5f},
        .diffusivity = {1.f, 1.f, 1.f},
        .specularity = {1.f, 1.f, 1.f},
        .node_name = "1234",
        .only_black = false,
        .shadow = false});
    (*this)(scene, rotate, scale, scene_graph_config);
}

void Render2::render_height_map(
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, 2>& normalization_matrix,
    bool rotate,
    float scale,
    const SceneGraphConfig& scene_graph_config,
    const CameraConfig& camera_config)
{
    const auto r = std::make_shared<RenderableHeightMap>(rgb_picture, height_picture, normalization_matrix);
    SceneNodeResources scene_node_resources;
    Scene scene;
    scene_node_resources.add_resource("RenderableHeightMap", r);
    auto on = new SceneNode;
    scene_node_resources.instantiate_renderable("RenderableHeightMap", "RenderableHeightMap", *on, SceneNodeResourceFilter());
    scene.add_root_node("obj", on);
    scene.add_root_node("camera", new SceneNode);
    scene.get_node("camera")->set_camera(std::make_shared<GenericCamera>(camera_config, GenericCamera::Mode::PERSPECTIVE));
    scene.add_root_node("light", new SceneNode);
    scene.get_node("light")->add_light(new Light{
        .ambience = {0.5f, 0.5f, 0.5f},
        .diffusivity = {1.f, 1.f, 1.f},
        .specularity = {1.f, 1.f, 1.f},
        .node_name = "1234",
        .only_black = false,
        .shadow = false});
    (*this)(scene, rotate, scale, scene_graph_config);
}

GLFWwindow* Render2::window() const {
    assert_true(window_.get());
    return window_->window();
}

bool Render2::window_should_close() const {
    return (window_ != nullptr) && GLFW_CHK(glfwWindowShouldClose(window_->window()));
}
