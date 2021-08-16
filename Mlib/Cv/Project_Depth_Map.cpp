#include "Project_Depth_Map.hpp"
#include <Mlib/Cv/Intrinsic_Matrix_Conversion.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/Cameras/Projection_Matrix_Camera.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Depth_Map_Resource.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

void Mlib::Cv::project_depth_map(
    const Array<float>& rgb_picture0,
    const Array<float>& depth_picture0,
    const TransformationMatrix<float, 2>& intrinsic_matrix0,
    const TransformationMatrix<float, 3>& ke_1_0,
    Array<float>& rgb_picture1,
    Array<float>& depth_picture1,
    const TransformationMatrix<float, 2>& intrinsic_matrix1,
    int width,
    int height,
    float z_near,
    float z_far)
{
    size_t num_renderings = 1;
    RenderConfig render_config{ .screen_width = width, .screen_height = height };
    RenderResults render_results;
    RenderedSceneDescriptor rsd;
    render_results.outputs[rsd] = { .with_depth_texture = true };

    Render2 render2{ render_config, num_renderings, &render_results };

    SceneNodeResources scene_node_resources;
    RenderingContextGuard rrg{scene_node_resources, "primary_rendering_resources", render_config.anisotropic_filtering_level, 0};
    SceneGraphConfig scene_graph_config;

    const auto r = std::make_shared<DepthMapResource>(rgb_picture0, depth_picture0, intrinsic_matrix0, 0.f);  // 0.f = z_offset
    scene_node_resources.add_resource("DepthMapResource", r);
    auto on = new SceneNode;
    scene_node_resources.instantiate_renderable("DepthMapResource", "DepthMapResource", *on, SceneNodeResourceFilter());

    Scene scene;
    scene.add_root_node("obj", on);
    scene.add_root_node("camera", new SceneNode);
    TransformationMatrix<float, 3> cpose = ke_1_0.inverted_scaled();
    float cscale = cpose.get_scale();
    scene.get_node("camera")->set_absolute_pose(cpose.t(), matrix_2_tait_bryan_angles(cpose.R() / cscale), cscale);
    scene.get_node("camera")->create_camera<ProjectionMatrixCamera>(
        opengl_matrix_from_hz_intrinsic_matrix(
            intrinsic_matrix1,
            (float)width,
            (float)height,
            z_near,
            z_far));

    SelectedCameras selected_cameras{ scene };
    selected_cameras.set_camera_node_name("camera");
    StandardCameraLogic camera_logic{ scene, selected_cameras };
    StandardRenderLogic render_logic{ scene, camera_logic, ClearMode::COLOR_AND_DEPTH };
    ReadPixelsLogic read_pixels_logic{ render_logic };
    render2(read_pixels_logic);
    
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
