#include "Render_Height_Map.hpp"
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Height_Map_Resource.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

#ifndef __ANDROID__

void Mlib::render_height_map(
    Render& render,
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
    auto on = make_unique_scene_node();
    scene_node_resources.instantiate_child_renderable(
        "HeightMapResource",
        ChildInstantiationOptions{
            .instance_name = VariableAndHash<std::string>{ "HeightMapResource" },
            .scene_node = on.ref(DP_LOC),
            .interpolation_mode = PoseInterpolationMode::DISABLED,
            .renderable_resource_filter = RenderableResourceFilter{}});
    std::unique_ptr<Camera> camera(new PerspectiveCamera(
        camera_config,
        PerspectiveCamera::Postprocessing::ENABLED));
    render.render_node(std::move(on), FixedArray<float, 3>{1.f, 0.f, 1.f}, rotate, scale, camera_z, scene_graph_config, std::move(camera));
}

#endif
