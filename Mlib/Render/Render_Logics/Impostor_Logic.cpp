#include "Impostor_Logic.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Square_Resource.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>

using namespace Mlib;

bool OriginalNodeHider::node_shall_be_hidden(
    const SceneNode& camera_node,
    const ExternalRenderPass& external_render_pass) const
{
    if (external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        return false;
    }
    return true;
}

bool ImpostorNodeHider::node_shall_be_hidden(
    const SceneNode& camera_node,
    const ExternalRenderPass& external_render_pass) const
{
    if (!is_initialized) {
        return true;
    }
    if (external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        return true;
    }
    return false;
}

ImpostorLogic::ImpostorLogic(
    RenderLogic& child_logic,
    Scene& scene,
    SceneNode& orig_node,
    SelectedCameras& cameras)
: child_logic_{child_logic},
  scene_{scene},
  orig_node_{orig_node},
  cameras_{cameras},
  rendering_context_{RenderingContextStack::resource_context()},
  old_camera_position_(NAN),
  old_dir_camera_to_renderable_(NAN)
{
    {
        std::string suffix = std::to_string(scene.get_uuid());
        texture_id_ = "impostor_color." + suffix;
        impostor_name_ = "impostor-" + suffix;
    }
    orig_node.set_node_hider(orig_hider);
    Material material{
        // .blend_mode = BlendMode::CONTINUOUS,
        .textures = { {.texture_descriptor = TextureDescriptor{.color = texture_id_}} },
        .ambience = OrderableFixedArray<float, 3>{2.f, 2.f, 2.f},
        .diffusivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
        .specularity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f}};
    material.compute_color_mode();
    SquareResource res{
        FixedArray<float, 2, 2>{-10.f, 0.f, 10.f, 5.f},
        TransformationMatrix<float, float, 3>::identity(),
        material};
    auto impostor_node = std::make_unique<SceneNode>();
    auto orig_node_trafo = orig_node.absolute_model_matrix();
    impostor_node->set_relative_pose(orig_node_trafo.t(), fixed_zeros<float, 3>(), 1.f);
    res.instantiate_renderable(InstantiationOptions{
        .instance_name = "impostor",
        .scene_node = *impostor_node,
        .renderable_resource_filter = RenderableResourceFilter()});
    impostor_node->set_node_hider(impostor_hider_);
    scene.add_root_node(impostor_name_, std::move(impostor_node));
}

ImpostorLogic::~ImpostorLogic() {
    if (fbs_ != nullptr) {
        // Warning in case of exception during child_logic_.render.
        rendering_context_.rendering_resources->delete_texture(texture_id_, DeletionFailureMode::WARN);
    }
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    if (!orig_node_.shutting_down()) {
        std::lock_guard lock{scene_.delete_node_mutex()};
        orig_node_.remove_child(impostor_name_);
    }
}

void ImpostorLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ImpostorLogic::render");
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        throw std::runtime_error("ImpostorLogic received wrong rendering");
    }
    auto camera_position = scene_.get_node(cameras_.camera_node_name()).absolute_model_matrix().t();
    auto renderable_position = orig_node_.absolute_model_matrix().t();
    auto dir_camera_to_renderable = renderable_position - camera_position;
    if (sum(squared(dir_camera_to_renderable)) < 1e-12) {
        return;
    }
    dir_camera_to_renderable /= std::sqrt(sum(squared(dir_camera_to_renderable)));
    if ((fbs_ == nullptr) ||
        (dot0d(dir_camera_to_renderable, old_dir_camera_to_renderable_) < 0.9))
    {
        old_dir_camera_to_renderable_ = dir_camera_to_renderable;
        GLsizei impostor_texture_width = 1024;
        GLsizei impostor_texture_height = 2048;
        ViewportGuard vg{0, 0, impostor_texture_width, impostor_texture_height};
        SceneNode impostor_camera_node;
        TransformationMatrix<float, double, 3> impostor_camera_model_matrix{
            gl_lookat_relative(dir_camera_to_renderable).casted<float>(),
            camera_position};
        impostor_camera_node.set_relative_pose(camera_position, matrix_2_tait_bryan_angles(impostor_camera_model_matrix.R()), 1.f);
        impostor_camera_node.set_camera(
            std::make_unique<GenericCamera>(
                CameraConfig(),
                GenericCamera::Postprocessing::DISABLED,
                GenericCamera::Mode::PERSPECTIVE));
        RenderedSceneDescriptor impostor_rsd{.external_render_pass = {ExternalRenderPassType::IMPOSTOR_NODE, "", &orig_node_, &impostor_camera_node}, .time_id = 0};
        if (fbs_ == nullptr) {
            fbs_ = std::make_unique<FrameBuffer>();
        }
        fbs_->configure({
            .width = impostor_texture_width,
            .height = impostor_texture_height,
            .with_depth_texture = false,
            .with_mipmaps = true,
            .nsamples_msaa = render_config.impostor_nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{*fbs_};
            RenderingContextGuard rrg{rendering_context_};
            AggregateRendererGuard arg{
                std::make_shared<AggregateArrayRenderer>(),
                std::make_shared<AggregateArrayRenderer>()};
            InstancesRendererGuard irg{
                std::make_shared<ArrayInstancesRenderers>(),
                std::make_shared<ArrayInstancesRenderer>()};
            RenderToScreenGuard rsg;
            CHK(glClearColor(1.f, 0.f, 1.f, 1.f));
            CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            // child_logic_.render(impostor_texture_width, impostor_texture_height, render_config, scene_graph_config, render_results, impostor_rsd);
            // VectorialPixels<float, 3> vpx{ArrayShape{size_t(impostor_texture_width), size_t(impostor_texture_height)}};
            // CHK(glReadPixels(0, 0, impostor_texture_width, impostor_texture_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
            // StbImage::from_float_rgb(vpx.to_array()).save_to_file("/tmp/impostor.png");
        }

        rendering_context_.rendering_resources->set_texture(texture_id_, fbs_->texture_color());
        impostor_hider_.is_initialized = true;
    }
}

float ImpostorLogic::near_plane() const {
    return child_logic_.near_plane();
}

float ImpostorLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<double, 4, 4>& ImpostorLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, double, 3>& ImpostorLogic::iv() const {
    return child_logic_.iv();
}

bool ImpostorLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void ImpostorLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ImpostorLogic\n";
    child_logic_.print(ostr, depth + 1);
}
