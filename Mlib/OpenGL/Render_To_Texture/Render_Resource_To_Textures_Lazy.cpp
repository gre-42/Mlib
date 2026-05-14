#include "Render_Resource_To_Textures_Lazy.hpp"
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/OpenGL/Render_Logics/Clear_Mode.hpp>
#include <Mlib/OpenGL/Render_Logics/Lambda_Render_Logic.hpp>
#include <Mlib/OpenGL/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/OpenGL/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/OpenGL/Render_To_Texture/Render_Logic_To_Textures_Lazy.hpp>
#include <Mlib/OpenGL/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Scene_Graph/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::render_resource_to_textures_lazy(
    VariableAndHash<std::string> resource,
    VariableAndHash<std::string> color_texture_name,
    VariableAndHash<std::string> depth_texture_name,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const OrthoCameraConfig& ortho_camera_config,
    FrameBufferChannelKind depth_kind,
    const FixedArray<int, 2>& texture_size,
    int nsamples_msaa,
    float dpi,
    ColorExtrapolationMode color_extrapolation_mode)
{
    auto lrl = std::make_shared<LambdaRenderLogic>(
        [&scene_node_resources, &rendering_resources, resource=std::move(resource), ortho_camera_config](
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id)
    {
        auto& gpu_object_factory = RenderingContextStack::primary_gpu_object_factory();
        auto& gpu_vertex_array_renderer = RenderingContextStack::primary_gpu_vertex_array_renderer();
        AggregateRendererGuard aggregate_renderer_guard{
            nullptr,
            nullptr,
            std::make_shared<AggregateArrayRenderer>(rendering_resources),
            std::make_shared<AggregateArrayRenderer>(rendering_resources)};
        InstancesRendererGuard instances_renderer_guard{
            nullptr,
            nullptr,
            std::make_shared<ArrayInstancesRenderers>(gpu_object_factory, gpu_vertex_array_renderer),
            std::make_shared<ArrayInstancesRenderer>(gpu_object_factory, gpu_vertex_array_renderer)};
        static const auto instance_name = VariableAndHash<std::string>{"inst"};
        static const auto camera_node_name = VariableAndHash<std::string>{"camera"};
        static const auto light_node_name = VariableAndHash<std::string>{"light"};
        Scene scene{"resource_to_textures"};
        scene_node_resources.instantiate_root_renderables(
            resource,
            RootInstantiationOptions{
                .rendering_resources = &rendering_resources,
                .instance_name = instance_name,
                .absolute_model_matrix = TransformationMatrix<SceneDir, ScenePos, 3>::identity(),
                .scene = scene,
                .renderable_resource_filter = RenderableResourceFilter{}});
        auto camera_node = make_unique_scene_node();
        camera_node->set_camera(std::make_unique<OrthoCamera>(ortho_camera_config, OrthoCamera::Postprocessing::DISABLED));
        RenderedSceneDescriptor billboard_scene_rsd{
            .external_render_pass = {
                frame_id.external_render_pass.observer,             // observer
                ExternalRenderPassType::BILLBOARD_SCENE,            // pass
                frame_id.external_render_pass.time,                 // time
                VariableAndHash<std::string>(),                     // black_node_name
                nullptr,                                            // singular_node
                camera_node.get(CURRENT_SOURCE_LOCATION)            // nonstandard_camera_node
            },
            .time_id = 0};
        {
            auto light_node = make_unique_scene_node();
            auto light = std::make_shared<Light>();
            light_node->add_light(std::move(light));
            scene.add_root_node(
                light_node_name,
                std::move(light_node),
                RenderingDynamics::MOVING,
                RenderingStrategies::OBJECT);
        }
        SelectedCameras selected_cameras{scene};
        StandardCameraLogic standard_camera_logic{
            scene,
            selected_cameras};
        StandardRenderLogic standard_render_logic{
            scene,                          // scene
            standard_camera_logic,          // child_logic
            fixed_zeros<float, 3>(),        // background_color
            ClearMode::COLOR_AND_DEPTH };   // clear_mode
        standard_render_logic.render_toplevel(
            lx, ly, render_config, scene_graph_config, render_results, billboard_scene_rsd);
    });
    render_logic_to_textures_lazy(
        std::move(lrl),
        rendering_resources,
        depth_kind,
        texture_size,
        nsamples_msaa,
        dpi,
        color_extrapolation_mode,
        std::move(color_texture_name),
        std::move(depth_texture_name));
}
