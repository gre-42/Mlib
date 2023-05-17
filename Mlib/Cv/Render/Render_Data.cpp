#include "Render_Data.hpp"
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Render/Resources/Depth_Map_Resource.hpp>
#include <Mlib/Cv/Render/Resources/Point_Cloud_Resource.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Cameras/Projection_Matrix_Camera.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Height_Map_Resource.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Particles_Resources.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

void Mlib::Cv::render_point_cloud(
    Render2& render,
    const Array<TransformationMatrix<float, float, 3>>& points,
    std::unique_ptr<Camera>&& camera,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config)
{
    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    const auto r = std::make_shared<PointCloudResource>(points);
    scene_node_resources.add_resource("PointCloudResource", r);
    auto on = std::make_unique<SceneNode>();
    scene_node_resources.instantiate_renderable(
        "PointCloudResource",
        InstantiationOptions{
            .instance_name = "PointCloudResource",
            .scene_node = *on,
            .renderable_resource_filter = RenderableResourceFilter{}});
    render.render_node(
        std::move(on),
        {1.f, 0.f, 1.f},
        rotate,
        scale,
        camera_z,
        scene_graph_config,
        std::move(camera));
}

void Mlib::Cv::render_depth_map(
    Render2& render,
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    float near_plane,
    float far_plane,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config)
{
    DepthMapPackage package{
        .time = std::chrono::milliseconds(0),
        .rgb = rgb_picture,
        .depth = depth_picture,
        .ki = intrinsic_matrix,
        .ke = TransformationMatrix<float, float, 3>::identity()};
    render_depth_maps(
        render,
        { package },
        Array<TransformationMatrix<float, float, 3>>{},
        { },
        { },
        intrinsic_matrix,
        TransformationMatrix<float, float, 3>::identity(),
        (float)depth_picture.shape(1),
        (float)depth_picture.shape(0),
        near_plane,
        far_plane,
        rotate,
        scale,
        camera_z,
        scene_graph_config);
}

void Mlib::Cv::render_depth_maps(
    Render2& render,
    const std::vector<DepthMapPackage>& packages,
    const Array<TransformationMatrix<float, float, 3>>& points,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& mesh,
    const std::vector<TransformationMatrix<float, float, 3>>& beacon_locations,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const TransformationMatrix<float, float, 3>& extrinsic_matrix,
    float width,
    float height,
    float near_plane,
    float far_plane,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    float point_radius,
    float cos_threshold)
{
    SceneNodeResources scene_node_resources;
    ParticlesResources particles_resources;
    auto rrg = RenderingContextGuard::root(
        scene_node_resources,
        particles_resources,
        "primary_rendering_resources",
        16,
        0);
    auto root_node = std::make_unique<SceneNode>();
    {
        size_t i = 0;
        for (const DepthMapPackage& package : packages) {
            std::string resource_name = "DepthMapResource_" + std::to_string(i++);
            const auto r = std::make_shared<DepthMapResource>(package.rgb, package.depth, package.ki, cos_threshold);
            scene_node_resources.add_resource(resource_name, r);
            auto on = std::make_unique<SceneNode>();
            TransformationMatrix<float, float, 3> cpos = cv_to_opengl_extrinsic_matrix(package.ke).inverted();
            float scale = cpos.get_scale();
            on->set_absolute_pose(cpos.t().casted<double>(), matrix_2_tait_bryan_angles(cpos.R() / scale), scale);
            scene_node_resources.instantiate_renderable(
                resource_name,
                InstantiationOptions{
                    .instance_name = "DepthMap",
                    .scene_node = *on,
                    .renderable_resource_filter = RenderableResourceFilter{}});
            root_node->add_child(resource_name, std::move(on));
        }
    }
    if (points.initialized() && (points.length() > 0)) {
        const auto r = std::make_shared<PointCloudResource>(points, point_radius);
        scene_node_resources.add_resource("PointCloudResource", r);
        scene_node_resources.instantiate_renderable(
            "PointCloudResource",
            InstantiationOptions{
                .instance_name = "DepthMap",
                .scene_node = *root_node,
                .renderable_resource_filter = RenderableResourceFilter{}});
    }
    if (!mesh.empty()) {
        std::list<std::shared_ptr<ColoredVertexArray<float>>> tmesh;
        for (const auto& m : mesh) {
            tmesh.push_back(m->transformed<float>(cv_to_opengl_matrix(), "_tmesh"));
        }
        const auto r = std::make_shared<ColoredVertexArrayResource>(tmesh);
        scene_node_resources.add_resource("ColoredVertexArrayResource", r);
        scene_node_resources.instantiate_renderable(
            "ColoredVertexArrayResource",
            InstantiationOptions{
                .instance_name = "ColoredVertexArray",
                .scene_node = *root_node,
                .renderable_resource_filter = RenderableResourceFilter{}});
    }
    std::vector<TransformationMatrix<float, double, 3>> transformed_beacon_locations;
    transformed_beacon_locations.reserve(beacon_locations.size());
    for (const auto& b : beacon_locations) {
        transformed_beacon_locations.push_back(cv_to_opengl_extrinsic_matrix(b).casted<float, double>());
    }
    if (!beacon_locations.empty()) {
        TriangleList<float> tl{ "beacon", Material(), PhysicsMaterial::ATTR_VISIBLE };
        tl.draw_rectangle_wo_normals(
            {-1.f, 1.f, 0.f},
            {1.f, 1.f, 0.f},
            {1.f, -1.f, 0.f},
            {-1.f, -1.f, 0.f});
        const auto r = std::make_shared<ColoredVertexArrayResource>(tl.triangle_array());
        scene_node_resources.add_resource("beacon", r);
        auto bn = std::make_unique<SceneNode>();
        scene_node_resources.instantiate_renderable(
            "beacon",
            InstantiationOptions{
                .instance_name = "beacon",
                .scene_node = *bn,
                .renderable_resource_filter = RenderableResourceFilter{}});
        root_node->add_child("beacon", std::move(bn));
    }
    std::unique_ptr<Camera> camera(new ProjectionMatrixCamera(cv_to_opengl_hz_intrinsic_matrix(
        intrinsic_matrix,
        width,
        height,
        near_plane,
        far_plane)));
    render.render_node(
        std::move(root_node),
        {1.f, 0.f, 1.f},
        rotate,
        scale,
        camera_z,
        scene_graph_config,
        std::move(camera),
        &transformed_beacon_locations);
}

void Mlib::Cv::render_height_map(
    Render2& render,
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, float, 2>& normalization_matrix,
    NormalType normal_type,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    const PerspectiveCameraConfig& camera_config)
{
    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    const auto r = std::make_shared<HeightMapResource>(rgb_picture, height_picture, normalization_matrix, normal_type);
    scene_node_resources.add_resource("HeightMapResource", r);
    auto on = std::make_unique<SceneNode>();
    scene_node_resources.instantiate_renderable(
        "HeightMapResource",
        InstantiationOptions{
            .instance_name = "HeightMapResource",
            .scene_node = *on,
            .renderable_resource_filter = RenderableResourceFilter{}});
    std::unique_ptr<Camera> camera(new PerspectiveCamera(
        camera_config,
        PerspectiveCamera::Postprocessing::ENABLED));
    render.render_node(
        std::move(on),
        {1.f, 0.f, 1.f},
        rotate,
        scale,
        camera_z,
        scene_graph_config,
        std::move(camera));
}
