#include "Render2.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Print_Gl_Version_Info.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Rotating_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Renderer.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifndef __ANDROID__

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
    std::atomic_size_t& num_renderings,
    SetFps& set_fps,
    RenderResults* render_results)
: num_renderings_{num_renderings},
  set_fps_{set_fps},
  render_results_{render_results},
  render_config_{render_config}
{
    if (glfwInit() == GLFW_FALSE) {
        THROW_OR_ABORT("glfwInit failed");
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
        TemporarilyIgnoreFloatingPointExeptions ignore_except;
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
        GLFW_CHK(glfwSetInputMode(&window_->glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED));
    }
    {
        GlContextGuard gcg{*window_};
        CHK(int version = gladLoadGL(glfwGetProcAddress));
        if (version == 0) {
            THROW_OR_ABORT("gladLoadGL failed");
        }
    }
}

Render2::~Render2() {
    {
        // This internally calls "execute_render_gc"
        GlContextGuard gcg{ *window_ };
    }
    window_.reset();
    GLFW_WARN(glfwTerminate());
}

void Render2::print_hardware_info() const {
    GlContextGuard gcg{ *window_ };
    print_gl_version_info();
}

Renderer Render2::generate_renderer() const
{
    return Renderer{*window_, render_config_, num_renderings_, set_fps_, render_results_};
}

void Render2::render(
    RenderLogic& logic,
    const SceneGraphConfig& scene_graph_config,
    ButtonStates* button_states,
    CursorStates* cursor_states,
    CursorStates* scroll_wheel_states) const
{
    generate_renderer().render_and_handle_events(
        logic,
        scene_graph_config,
        button_states,
        cursor_states,
        scroll_wheel_states);
}

void Render2::render_scene(
    const Scene& scene,
    const FixedArray<float, 3>& background_color,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations) const
{
    ButtonStates button_states;
    RotatingLogic rotating_logic{
        button_states,
        window_->glfw_window(),
        scene,
        rotate,
        scale,
        camera_z,
        background_color,
        beacon_locations};
    ReadPixelsLogic read_pixels_logic{ rotating_logic };
    render(read_pixels_logic, scene_graph_config, &button_states);
}

void Render2::render_node(
    DanglingUniquePtr<SceneNode>&& node,
    const FixedArray<float, 3>& background_color,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    std::unique_ptr<Camera>&& camera,
    const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations) const
{
    DeleteNodeMutex delete_node_mutex;
    Scene scene{ delete_node_mutex };
    DestructionGuard scene_destruction_guard{[&](){
        std::scoped_lock lock{ delete_node_mutex };
        scene.shutdown();
    }};
    scene.add_root_node("obj", std::move(node));
    scene.add_root_node("camera", make_dunique<SceneNode>());
    // std::make_shared<GenericCamera>(camera_config, GenericCamera::Postprocessing::ENABLED, GenericCamera::Mode::PERSPECTIVE)
    scene.get_node("camera", DP_LOC)->set_camera(std::move(camera));
    scene.add_root_node("light", make_dunique<SceneNode>());
    scene.get_node("light", DP_LOC)->add_light(std::make_unique<Light>(Light{
        .ambience = {0.5f, 0.5f, 0.5f},
        .diffusivity = {1.f, 1.f, 1.f},
        .specularity = {1.f, 1.f, 1.f},
        .shadow_render_pass = ExternalRenderPassType::NONE}));
    render_scene(scene, background_color, rotate, scale, camera_z, scene_graph_config, beacon_locations);
}

GLFWwindow& Render2::glfw_window() const {
    assert_true(window_.get());
    return window_->glfw_window();
}

IWindow& Render2::window() const {
    assert_true(window_.get());
    return *window_;
}

bool Render2::window_should_close() const {
    return (window_ != nullptr) && GLFW_CHK(glfwWindowShouldClose(&window_->glfw_window()));
}

#endif
