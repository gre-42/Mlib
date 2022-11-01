#include "Project_Depth_Map.hpp"
#include <Mlib/Cv/Render/Resources/Depth_Map_Resource.hpp>
#include <Mlib/Geometry/Cameras/Projection_Matrix_Camera.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

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
    RenderResults render_results;
    RenderedSceneDescriptor rsd;
    render_results.outputs[rsd] = { .with_depth_texture = true };

    Render2 render2{ render_config, num_renderings, &render_results };

    SceneNodeResources scene_node_resources;
    auto rrg = RenderingContextGuard::root(
        scene_node_resources,
        "primary_rendering_resources",
        render_config.anisotropic_filtering_level,
        0);
    SceneGraphConfig scene_graph_config;

    const auto r = std::make_shared<DepthMapResource>(rgb_picture0, depth_picture0, intrinsic_matrix0);
    scene_node_resources.add_resource("DepthMapResource", r);
    auto on = std::make_unique<SceneNode>();
    scene_node_resources.instantiate_renderable(
        "DepthMapResource",
        InstantiationOptions{
            .instance_name = "DepthMapResource",
            .scene_node = *on,
            .renderable_resource_filter = RenderableResourceFilter()});

    DeleteNodeMutex delete_node_mutex;
    Scene scene{ delete_node_mutex };
    scene.add_root_node("obj", std::move(on));
    scene.add_root_node("camera", std::make_unique<SceneNode>());
    TransformationMatrix<float, float, 3> cpose = cv_to_opengl_extrinsic_matrix(ke_1_0).inverted();
    float cscale = cpose.get_scale();
    scene.get_node("camera").set_absolute_pose(cpose.t().casted<double>(), matrix_2_tait_bryan_angles(cpose.R() / cscale), cscale);
    scene.get_node("camera").set_camera(std::make_unique<ProjectionMatrixCamera>(
        cv_to_opengl_hz_intrinsic_matrix(
            intrinsic_matrix1,
            (float)width,
            (float)height,
            z_near,
            z_far)));

    SelectedCameras selected_cameras{ scene };
    selected_cameras.set_camera_node_name("camera");
    StandardCameraLogic camera_logic{ scene, selected_cameras, delete_node_mutex };
    StandardRenderLogic render_logic{ scene, camera_logic, {1.f, 0.f, 1.f}, ClearMode::COLOR_AND_DEPTH };
    ReadPixelsLogic read_pixels_logic{ render_logic };
    render2.render(read_pixels_logic);
    
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
