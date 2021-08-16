#include "Render_Data.hpp"
#include <Mlib/Cv/Intrinsic_Matrix_Conversion.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Cameras/Projection_Matrix_Camera.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Depth_Map_Resource.hpp>
#include <Mlib/Render/Resources/Height_Map_Resource.hpp>
#include <Mlib/Render/Resources/Point_Cloud_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::render_point_cloud(
    Render2& render,
    const Array<FixedArray<float, 3>>& points,
    std::unique_ptr<Camera>& camera,
    bool rotate,
    float scale,
    const SceneGraphConfig& scene_graph_config)
{
    auto& scene_node_resources = RenderingContextStack::primary_rendering_resources()->scene_node_resources();
    const auto r = std::make_shared<PointCloudResource>(points);
    scene_node_resources.add_resource("PointCloudResource", r);
    auto on = new SceneNode;
    scene_node_resources.instantiate_renderable("PointCloudResource", "PointCloudResource", *on, SceneNodeResourceFilter());
    render.render_node(*on, rotate, scale, scene_graph_config, camera);
}

void Mlib::render_depth_map(
    Render2& render,
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    float near_plane,
    float far_plane,
    float z_offset,
    bool rotate,
    float scale,
    const SceneGraphConfig& scene_graph_config)
{
    SceneNodeResources scene_node_resources;
    RenderingContextGuard rrg{
        scene_node_resources,
        "primary_rendering_resources",
        16,
        0};
    const auto r = std::make_shared<DepthMapResource>(rgb_picture, depth_picture, intrinsic_matrix, z_offset);
    scene_node_resources.add_resource("DepthMapResource", r);
    auto on = new SceneNode;
    scene_node_resources.instantiate_renderable("DepthMapResource", "DepthMapResource", *on, SceneNodeResourceFilter());
    std::unique_ptr<Camera> camera(new ProjectionMatrixCamera(Cv::opengl_matrix_from_hz_intrinsic_matrix(
        intrinsic_matrix,
        (float)depth_picture.shape(1),
        (float)depth_picture.shape(0),
        near_plane,
        far_plane)));
    render.render_node(
        *on,
        rotate,
        scale,
        scene_graph_config,
        camera);
}

void Mlib::render_height_map(
    Render2& render,
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, 2>& normalization_matrix,
    bool rotate,
    float scale,
    const SceneGraphConfig& scene_graph_config,
    const CameraConfig& camera_config)
{
    auto& scene_node_resources = RenderingContextStack::primary_rendering_resources()->scene_node_resources();
    const auto r = std::make_shared<HeightMapResource>(rgb_picture, height_picture, normalization_matrix);
    scene_node_resources.add_resource("HeightMapResource", r);
    auto on = new SceneNode;
    scene_node_resources.instantiate_renderable("HeightMapResource", "HeightMapResource", *on, SceneNodeResourceFilter());
    std::unique_ptr<Camera> camera(new GenericCamera(camera_config, GenericCamera::Mode::PERSPECTIVE));
    render.render_node(*on, rotate, scale, scene_graph_config, camera);
}
