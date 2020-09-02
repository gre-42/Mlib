#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Render2.hpp"

#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Fps.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/linmath.hpp>
#include <Mlib/Render/Render_Logics/Rotating_Logic.hpp>
#include <Mlib/Render/Renderables/Renderable_Depth_Map.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Set_Fps.hpp>
#include <fenv.h>
#include <iostream>

using namespace Mlib;

static void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

/*static void print_mat4x4(const mat4x4& m) {
    for(size_t r = 0; r < 4; ++r) {
        for(size_t c = 0; c < 4; ++c) {
            std::cerr << m[r][c] << " ";
        }
        std::cerr << std::endl;
    }
}*/

Render2::Render2(
    bool& leave_render_loop,
    RenderResults* render_results,
    const RenderConfig& render_config)
: leave_render_loop_{leave_render_loop},
  render_results_{render_results},
  render_config_{render_config}
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    if (render_results != nullptr && (render_results->output != nullptr || !render_results->outputs.empty())) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
    if (render_config.nsamples_msaa != 1) {
        glfwWindowHint(GLFW_SAMPLES, render_config.nsamples_msaa);
    }
    if (render_config.window_maximized) {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }
#ifndef WIN32
    int fpeflags = fegetexcept();
    fedisableexcept(FE_ALL_EXCEPT);
#endif
    if (window_ != nullptr) {
        throw std::runtime_error("Multiple calls to render2");
    }
    window_ = std::make_unique<Window>(
        render_config.screen_width,
        render_config.screen_height,
        render_config.window_title.c_str(),
        render_config.full_screen ? glfwGetPrimaryMonitor() : nullptr,
        nullptr);
#ifndef WIN32
    feenableexcept(fpeflags);
#endif
    if (!render_config.show_mouse_cursor) {
        GLFW_CHK(glfwSetInputMode(window_->window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED));
    }
    glfwMakeContextCurrent(window_->window());
    CHK(int version = gladLoadGL(glfwGetProcAddress));
    if (version == 0) {
        throw std::runtime_error("gladLoadGL failed");
    }
    GLFW_CHK(glfwSwapInterval(render_config.swap_interval));
}

Render2::~Render2() {
    window_.release();
    GLFW_WARN(glfwTerminate());
}

void Render2::operator () (
    RenderLogic& logic,
    std::shared_mutex& mutex,
    const SceneGraphConfig& scene_graph_config)
{
    logic.initialize(window_->window());

    SetFps set_fps;
    Fps fps;
    size_t fps_i = 0;
    size_t fps_i_max = 500;
    size_t time_id = 0;
    // Get current keyboard inputs in case the scene was reloaded.
    GLFW_CHK(glfwPollEvents());
    while (!glfwWindowShouldClose(window_->window()) && !leave_render_loop_)
    {
        int width, height;

        GLFW_CHK(glfwGetFramebufferSize(window_->window(), &width, &height));

        CHK(glViewport(0, 0, width, height));

        {
            std::shared_lock lock{mutex};

            logic.render(
                width,
                height,
                render_config_,
                scene_graph_config,
                render_results_,
                (render_results_ != nullptr) && (!render_results_->outputs.empty())
                    ? RenderedSceneDescriptor{external_render_pass: {ExternalRenderPass::STANDARD_WITH_POSTPROCESSING, ""}, time_id: time_id, light_resource_id: 0}
                    : RenderedSceneDescriptor{external_render_pass: {ExternalRenderPass::UNDEFINED, ""}, time_id: time_id, light_resource_id: 0});
        }

        if (render_results_ != nullptr && render_results_->output != nullptr) {
            VectorialPixels<float, 3> vp{ArrayShape{size_t(height), size_t(width)}};
            CHK(glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, vp->flat_iterable().begin()));
            glfwSetWindowShouldClose(window_->window(), GLFW_TRUE);
            *render_results_->output = reverted_axis(vp.to_array(), 1);
        }
        if (render_results_ != nullptr && !render_results_->outputs.empty()) {
            glfwSetWindowShouldClose(window_->window(), GLFW_TRUE);
        }
        if (render_config_.dt != 0) {
            set_fps.tick(render_config_.dt, render_config_.max_residual_time, false); // false = print_residual_time
        } else if (render_config_.motion_interpolation) {
            throw std::runtime_error("Motion interpolation requires render_dt");
        }
        if (render_config_.print_fps) {
            fps_i = (fps_i + 1) % fps_i_max;
            fps.tick();
            if (fps_i == 0) {
                std::cerr << fps.fps() << " FPS" << std::endl;
            }
        }

        GLFW_CHK(glfwSwapBuffers(window_->window()));
        GLFW_CHK(glfwPollEvents());

        if (render_config_.motion_interpolation) {
            time_id = (time_id + 1) % 4;
        }
    }
}

void Render2::operator () (
    const Scene& scene,
    bool rotate,
    float scale,
    const SceneGraphConfig& scene_graph_config)
{
    RotatingLogic rotating_logic{scene, rotate, scale};
    std::shared_mutex mutex;
    (*this)(
        rotating_logic,
        mutex,
        scene_graph_config);
}

void Render2::operator () (
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const Array<float>& intrinsic_matrix,
    bool rotate,
    float scale,
    const SceneGraphConfig& scene_graph_config,
    const CameraConfig& camera_config)
{
    const auto r = std::make_shared<RenderableDepthMap>(rgb_picture, depth_picture, intrinsic_matrix);
    SceneNodeResources scene_node_resources;
    Scene scene;
    scene_node_resources.add_resource("DenderableDepthMap", r);
    auto on = new SceneNode;
    scene_node_resources.instantiate_renderable("DenderableDepthMap", "DenderableDepthMap", *on, SceneNodeResourceFilter{});
    scene.add_root_node("obj", on);
    scene.add_root_node("camera", new SceneNode);
    scene.get_node("camera")->set_camera(std::make_shared<GenericCamera>(camera_config, GenericCamera::Mode::PERSPECTIVE));
    (*this)(scene, rotate, scale, scene_graph_config);
}

bool Render2::window_should_close() const {
    return (window_ != nullptr) && glfwWindowShouldClose(window_->window());
}
