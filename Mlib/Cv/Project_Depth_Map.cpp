#include "Project_Depth_Map.hpp"
#include <Mlib/Cv/Render/Resources/Depth_Map_Resource.hpp>
#include <Mlib/Geometry/Cameras/Projection_Matrix_Camera.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

void Mlib::Cv::project_depth_map(
    const Array<float>& rgb_picture0,
    const Array<float>& depth_picture0,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix0,
    const TransformationMatrix<float, float, 3>& ke_1_0,
    Array<float>& rgb_picture1,
    Array<float>& depth_picture1,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix1,
    int width,
    int height,
    float z_near,
    float z_far)
{
    std::atomic_size_t num_renderings = 1;
    RenderConfig render_config{ .windowed_width = width, .windowed_height = height };
    InputConfig input_config;
    RenderResults render_results;
    RenderedSceneDescriptor rsd;
    render_results.outputs[rsd] = { .depth_kind = FrameBufferChannelKind::TEXTURE };
    SetFps set_fps{ nullptr };
    Render render{
        render_config,
        input_config,
        num_renderings,
        set_fps,
        [](){ return std::chrono::steady_clock::now(); },
        &render_results };

    SceneNodeResources scene_node_resources;
    ParticleResources particle_resources;
    TrailResources trail_resources;
    RenderingResources rendering_resources{
        "primary_rendering_resources",
        16 };
    RenderingContext rendering_context{
        scene_node_resources,
        particle_resources,
        trail_resources,
        rendering_resources };
    RenderingContextGuard rrg{ rendering_context };
    SceneGraphConfig scene_graph_config;

    const auto r = std::make_shared<DepthMapResource>(rgb_picture0, depth_picture0, intrinsic_matrix0);
    scene_node_resources.add_resource("DepthMapResource", r);
    auto on = make_dunique<SceneNode>();
    scene_node_resources.instantiate_child_renderable(
        "DepthMapResource",
        ChildInstantiationOptions{
            .instance_name = VariableAndHash<std::string>{ "DepthMapResource" },
            .scene_node = on.ref(CURRENT_SOURCE_LOCATION),
            .renderable_resource_filter = RenderableResourceFilter{}});

    DeleteNodeMutex delete_node_mutex;
    Scene scene{ "DepthMapScene", delete_node_mutex };
    scene.add_root_node("obj", std::move(on), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
    scene.add_root_node("camera", make_dunique<SceneNode>(), RenderingDynamics::MOVING, RenderingStrategies::OBJECT);
    TransformationMatrix<float, float, 3> cpose = cv_to_opengl_extrinsic_matrix(ke_1_0).inverted();
    float cscale = cpose.get_scale();
    scene.get_node("camera", DP_LOC)->set_absolute_pose(cpose.t.casted<double>(), matrix_2_tait_bryan_angles(cpose.R / cscale), cscale, std::chrono::steady_clock::now());
    scene.get_node("camera", DP_LOC)->set_camera(std::make_unique<ProjectionMatrixCamera>(
        cv_to_opengl_hz_intrinsic_matrix(
            intrinsic_matrix1,
            (float)width,
            (float)height,
            z_near,
            z_far)));

    SelectedCameras selected_cameras{ scene };
    selected_cameras.set_camera_node_name("camera");
    StandardCameraLogic camera_logic{ scene, selected_cameras };
    StandardRenderLogic render_logic{ scene, camera_logic, {1.f, 0.f, 1.f}, ClearMode::COLOR_AND_DEPTH };
    ButtonStates button_states;
    ReadPixelsLogic read_pixels_logic{ render_logic, button_states, ReadPixelsRole::NONE };
    render.render(read_pixels_logic, []() {});
    
    rgb_picture1 = render_results.outputs[rsd].rgb;
    // From: https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
    Array<float> z_b = render_results.outputs[rsd].depth;
    Array<float> z_n = 2.f * z_b - 1.f;
    Array<float> z_e = 2.f * z_near * z_far / (z_far + z_near - z_n * (z_far - z_near));
    depth_picture1 = z_e;
    for (size_t r = 0; r < depth_picture1.shape(0); ++r) {
        for (size_t c = 0; c < depth_picture1.shape(1); ++c) {
            if (z_b(r, c) == 1) {
                depth_picture1(r, c) = NAN;
                for (size_t color = 0; color < rgb_picture1.shape(0); ++color) {
                    rgb_picture1(color, r, c) = NAN;
                }
            }
        }
    }
}
