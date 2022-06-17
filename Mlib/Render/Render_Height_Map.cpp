#include "Render_Height_Map.hpp"
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Height_Map_Resource.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::render_height_map(
    Render2& render,
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, float, 2>& normalization_matrix,
    NormalType normal_type,
    bool rotate,
    float scale,
    float camera_z,
    const SceneGraphConfig& scene_graph_config,
    const CameraConfig& camera_config)
{
    auto& scene_node_resources = RenderingContextStack::primary_rendering_resources()->scene_node_resources();
    const auto r = std::make_shared<HeightMapResource>(rgb_picture, height_picture, normalization_matrix, normal_type);
    scene_node_resources.add_resource("HeightMapResource", r);
    auto on = std::make_unique<SceneNode>();
    scene_node_resources.instantiate_renderable("HeightMapResource", "HeightMapResource", *on, RenderableResourceFilter());
    std::unique_ptr<Camera> camera(new GenericCamera(camera_config, GenericCamera::Mode::PERSPECTIVE));
    render.render_node(std::move(on), FixedArray<float, 3>{1.f, 1.f, 1.f}, rotate, scale, camera_z, scene_graph_config, std::move(camera));
}
