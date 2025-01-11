#include "Render.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
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
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifndef __ANDROID__

using namespace Mlib;

static void error_callback(int error, const char* description)
{
    lerr() << "GLFW: " << description;
}

Render::Render(
    const RenderConfig& render_config,
    const InputConfig& input_config,
    std::atomic_size_t& num_renderings,
    SetFps& set_fps,
    std::function<std::chrono::steady_clock::time_point()> frame_time,
    RenderResults* render_results)
    : num_renderings_{ num_renderings }
    , set_fps_{ set_fps }
    , frame_time_{ std::move(frame_time) }
    , render_results_{ render_results }
    , render_config_{ render_config }
    , input_config_{ input_config }
{
    if (!frame_time_) {
        THROW_OR_ABORT("frame_time not set");
    }
    if (glfwInit() == GLFW_FALSE) {
        THROW_OR_ABORT("glfwInit failed");
    }
    GLFW_CHK(glfwSetErrorCallback(error_callback));

    // From: https://stackoverflow.com/questions/46510889
    // Not specifying GLFW_CONTEXT_VERSION_MAJOR/MINOR will select the highest supported version.
    // The actual version can then be obtained using "glGetIntegerv(GL_MAJOR_VERSION/MINOR...)
    // GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, render_config.opengl_major_version));
    // GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, render_config.opengl_minor_version));
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
            render_config.swap_interval,
            render_config.fullscreen_refresh_rate);
    }
    if (!render_config.show_mouse_cursor) {
        GLFW_CHK(glfwSetInputMode(&window_->glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED));
    }
    {
        GlContextGuard gcg{ *window_ };
        if (int version = gladLoadGL(glfwGetProcAddress); version == 0) {
            THROW_OR_ABORT("gladLoadGL failed");
        }
    }
}

Render::~Render() {
    if (window_ == nullptr) {
        verbose_abort("Render::~Render has null window");
    }
    {
        // This internally calls "execute_render_gc"
        GlContextGuard gcg{ *window_ };
    }
    window_ = nullptr;
    GLFW_ABORT(glfwTerminate());
}

void Render::print_hardware_info() const {
    {
        GlContextGuard gcg{ *window_ };
        print_gl_version_info();
    }
// #ifndef __ANDROID__
//     print_monitor_info();
// #endif
}

Renderer Render::generate_renderer() const
{
    return Renderer{ *window_, render_config_, input_config_, num_renderings_, set_fps_, frame_time_, render_results_ };
}

void Render::render(
    RenderLogic& logic,
    const std::function<void()>& event_callback,
    const SceneGraphConfig& scene_graph_config,
    ButtonStates* button_states,
    CursorStates* cursor_states,
    CursorStates* scroll_wheel_states) const
{
    generate_renderer().render_and_handle_events(
        logic,
        event_callback,
        scene_graph_config,
        button_states,
        cursor_states,
        scroll_wheel_states);
}

void Render::render_scene(
    const Scene& scene,
    const FixedArray<float, 3>& background_color,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    const std::vector<TransformationMatrix<float, ScenePos, 3>>* beacon_locations) const
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
    ReadPixelsLogic read_pixels_logic{
        rotating_logic,
        button_states,
        ReadPixelsRole::INTERMEDIATE | ReadPixelsRole::SCREENSHOT };
    render(read_pixels_logic, []() {}, scene_graph_config, &button_states);
}

void Render::render_node(
    DanglingUniquePtr<SceneNode>&& node,
    const FixedArray<float, 3>& background_color,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    std::unique_ptr<Camera>&& camera,
    const std::vector<TransformationMatrix<float, ScenePos, 3>>* beacon_locations) const
{
    DeleteNodeMutex delete_node_mutex;
    Scene scene{ "main_scene", delete_node_mutex };
    DestructionGuard scene_destruction_guard{[&](){
        scene.shutdown();
    }};
    scene.auto_add_root_node("obj", std::move(node), RenderingDynamics::MOVING);
    scene.add_root_node("camera", make_unique_scene_node(), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
    // std::make_shared<GenericCamera>(camera_config, GenericCamera::Postprocessing::ENABLED, GenericCamera::Mode::PERSPECTIVE)
    scene.get_node("camera", DP_LOC)->set_camera(std::move(camera));
    scene.add_root_node("light", make_unique_scene_node(), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
    scene.get_node("light", DP_LOC)->add_light(std::make_unique<Light>(Light{
        .ambient = {0.5f, 0.5f, 0.5f},
        .diffuse = {1.f, 1.f, 1.f},
        .specular = {1.f, 1.f, 1.f},
        .shadow_render_pass = ExternalRenderPassType::NONE}));
    render_scene(scene, background_color, rotate, scale, camera_z, scene_graph_config, beacon_locations);
}

GLFWwindow& Render::glfw_window() const {
    assert_true(window_ != nullptr);
    return window_->glfw_window();
}

IWindow& Render::window() const {
    assert_true(window_ != nullptr);
    return *window_;
}

bool Render::window_should_close() const {
    assert_true(window_ != nullptr);
    return window_->close_requested();
}

#endif
