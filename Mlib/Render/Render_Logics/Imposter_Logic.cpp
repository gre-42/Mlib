#include "Imposter_Logic.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Cameras/Frustum_Camera.hpp>
#include <Mlib/Geometry/Cameras/Frustum_Camera_Config.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera_Config.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At_Aabb.hpp>
#include <Mlib/Geometry/Coordinates/Npixels_For_Dpi.hpp>
#include <Mlib/Geometry/Intersection/Frustum3.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Imposter_Parameters.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Square_Resource.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

OriginalNodeHider::OriginalNodeHider(ImposterLogic& imposter_logic)
: imposter_logic_{imposter_logic}
{}

bool OriginalNodeHider::node_shall_be_hidden(
    const SceneNode& camera_node,
    const ExternalRenderPass& external_render_pass) const
{
    if (external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        return false;
    }
    if (imposter_logic_.imposter_node_ == nullptr) {
        return false;
    }
    return true;
}

bool ImposterNodeHider::node_shall_be_hidden(
    const SceneNode& camera_node,
    const ExternalRenderPass& external_render_pass) const
{
    if (external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        return true;
    }
    return false;
}

ImposterLogic::ImposterLogic(
    RenderLogic& child_logic,
    Scene& scene,
    SceneNode& orig_node,
    SelectedCameras& cameras,
    const std::string& debug_prefix,
    uint32_t max_texture_size,
    float down_sampling,
    float max_deviation,
    float min_distance)
: child_logic_{child_logic},
  scene_{scene},
  orig_node_{orig_node},
  cameras_{cameras},
  rendering_context_{RenderingContextStack::resource_context()},
  orig_hider{*this},
  imposter_node_{nullptr},
  debug_prefix_{debug_prefix},
  max_texture_size_{max_texture_size},
  down_sampling_{down_sampling},
  max_deviation_{max_deviation},
  min_distance_{min_distance}
{
    if ((max_texture_size_ < 1) || (max_texture_size_ > 4096)) {
        THROW_OR_ABORT("Imposter texture size out of bounds");
    }
    texture_id_ = "imposter_color" + scene.get_temporary_instance_suffix();
    auto aabb = orig_node_.relative_aabb();
    if (!aabb.has_value()) {
        THROW_OR_ABORT("Cannot compute AABB of \"" + debug_prefix_ + '"');
    }
    obj_relative_aabb_ = aabb.value();
    orig_node.set_node_hider(orig_hider);
}

ImposterLogic::~ImposterLogic() {
    if (fbs_ != nullptr) {
        // Warning in case of exception during child_logic_.render.
        rendering_context_.rendering_resources->delete_texture(texture_id_, DeletionFailureMode::WARN);
    }
    delete_imposter_if_exists();
}

void ImposterLogic::add_imposter(
    const ImposterParameters& ips,
    const FixedArray<double, 3>& orig_node_position,
    double camera_y,
    float angle_y)
{
    assert_true(imposter_node_ == nullptr);
    RenderingContextGuard rrg{rendering_context_};
    Material material{
        // .blend_mode = BlendMode::SEMI_CONTINUOUS_08,  // does not work with vegetation
        .blend_mode = BlendMode::BINARY_08,
        .textures = { {.texture_descriptor = TextureDescriptor{.color = texture_id_, .color_mode = ColorMode::RGBA}} },
        .ambience = OrderableFixedArray<float, 3>{2.f, 2.f, 2.f},
        .diffusivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
        .specularity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f}};
    material.compute_color_mode();
    SquareResource res{
        FixedArray<float, 2, 2>{ips.pos.min()(0), ips.pos.min()(1), ips.pos.max()(0), ips.pos.max()(1)},
        FixedArray<float, 2, 2>{ips.uv.min()(0), ips.uv.min()(1), ips.uv.max()(0), ips.uv.max()(1)},
        TransformationMatrix<float, float, 3>::identity(),
        material};
    auto new_imposter_node = std::make_unique<SceneNode>();
    new_imposter_node->set_relative_pose(
        FixedArray<double, 3>{orig_node_position(0), camera_y, orig_node_position(2)},
        FixedArray<float, 3>{0.f, angle_y, 0.f},
        1.f);
    res.instantiate_renderable(InstantiationOptions{
        .instance_name = "imposter",
        .scene_node = *new_imposter_node,
        .renderable_resource_filter = RenderableResourceFilter{}});
    new_imposter_node->set_node_hider(imposter_hider_);
    scene_.add_root_imposter_node(new_imposter_node.get());
    imposter_node_ = std::move(new_imposter_node);
}

void ImposterLogic::delete_imposter_if_exists() {
    if (imposter_node_ != nullptr) {
        scene_.delete_root_imposter_node(*imposter_node_);
        imposter_node_ = nullptr;
    }
}

void ImposterLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ImposterLogic::render");
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        THROW_OR_ABORT("ImposterLogic received wrong rendering");
    }
    auto& camera_node = scene_.get_node(cameras_.camera_node_name());
    auto v = camera_node.absolute_view_matrix();
    auto m = orig_node_.absolute_model_matrix();
    {
        auto cam_cp = camera_node.get_camera().copy();
        cam_cp->set_aspect_ratio(lx.flength() / ly.flength());
        auto mvp = dot2d(cam_cp->projection_matrix(), (v * m).casted<float, float>().affine());
        VisibilityCheck<float> vc{mvp};
        if (vc.orthographic()) {
            delete_imposter_if_exists();
            return;
        }
        auto frustum = Frustum3<float>::from_projection_matrix(mvp);
        if (!frustum.contains(obj_relative_aabb_)) {
            return;
        }
    }
    auto camera_position = camera_node.absolute_model_matrix().t();
    auto cam_to_obj2 = FixedArray<double, 2>{
        m.t()(0) - camera_position(0),
        m.t()(2) - camera_position(2)};
    auto cam_to_obj2_len2 = sum(squared(cam_to_obj2));
    if (cam_to_obj2_len2 < squared(min_distance_)) {
        delete_imposter_if_exists();
        return;
    }
    auto cam_to_obj2_len = std::sqrt(cam_to_obj2_len2);
    cam_to_obj2 /= cam_to_obj2_len;

    float dpi = PerspectiveCameraConfig().dpi(ly.flength()) / down_sampling_;

    bool imposter_outdated;
    if (imposter_node_ != nullptr) {
        auto mv = (v * m).casted<float, float>();
        size_t i = 0;
        imposter_outdated = !obj_relative_aabb_.for_each_corner([&](const FixedArray<float, 3>& corner){
            auto pc = mv.transform(corner);
            auto pc_old = v.transform(old_projected_bbox_(i)).casted<float>();
            if ((pc(2) > -1e-12) || (pc_old(2) > -1e-12)) {
                return true;
            }
            auto pc_proj = FixedArray<float, 2>{pc(0), pc(1)} / (-pc(2));
            auto pc_old_proj = FixedArray<float, 2>{pc_old(0), pc_old(1)} / (-pc_old(2));
            if (sum(squared(pc_proj - pc_old_proj)) > squared(max_deviation_ / dpi)) {
                return false;
            }
            ++i;
            return true;
        });
    } else {
        imposter_outdated = true;
    }
    if (imposter_outdated) {
        delete_imposter_if_exists();
        auto la = gl_lookat_aabb(
            camera_position,
            m,
            obj_relative_aabb_);
        if (!la.has_value()) {
            return;
        }

        {
            auto iv = TransformationMatrix<float, double, 3>(
                la.value().extrinsic_R, camera_position);
            auto mv = (TransformationMatrix<float, double, 3>::inverse(
                la.value().extrinsic_R, camera_position) *
                m).casted<float, float>();
            size_t i = 0;
            if (!obj_relative_aabb_.for_each_corner([&](const FixedArray<float, 3>& corner){
                auto pc = mv.transform(corner);
                if (pc(2) > -1e-12) {
                    return false;
                }
                auto pc_proj = pc / (-pc(2));
                old_projected_bbox_(i) = iv.transform(pc_proj.casted<double>() * cam_to_obj2_len);
                ++i;
                return true;
            }))
            {
                return;
            }
        }

        auto npixels = npixels_for_dpi(
            la.value().sensor_aabb,
            dpi,
            1,
            max_texture_size_);
        if (!npixels.has_value()) {
            return;
        }
        SceneNode imposter_camera_node;
        imposter_camera_node.set_relative_pose(camera_position, matrix_2_tait_bryan_angles(la.value().extrinsic_R), 1.f);
        imposter_camera_node.set_camera(
            std::make_unique<FrustumCamera>(
                FrustumCameraConfig::from_sensor_aabb(
                    npixels.value().scaled_sensor_aabb,
                    la.value().near_plane,
                    la.value().far_plane),
                FrustumCamera::Postprocessing::ENABLED));
        RenderedSceneDescriptor imposter_rsd{.external_render_pass = {ExternalRenderPassType::IMPOSTER_NODE, "", &orig_node_, &imposter_camera_node}, .time_id = 0};
        if (fbs_ == nullptr) {
            fbs_ = std::make_unique<FrameBuffer>();
        }
        ViewportGuard vg{npixels.value().width, npixels.value().height};
        fbs_->configure({
            .width = npixels.value().width,
            .height = npixels.value().height,
            .color_internal_format = GL_RGBA,
            .color_format = GL_RGBA,
            .nsamples_msaa = render_config.imposter_nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{*fbs_};
            RenderingContextGuard rrg{rendering_context_};
            AggregateRendererGuard arg{
                std::make_shared<AggregateArrayRenderer>(),
                std::make_shared<AggregateArrayRenderer>()};
            InstancesRendererGuard irg{
                std::make_shared<ArrayInstancesRenderers>(),
                std::make_shared<ArrayInstancesRenderer>()};
            // RenderToScreenGuard rsg;
            // CHK(glClearColor(1.f, 0.f, 1.f, 1.f));
            // CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            child_logic_.render(
                LayoutConstraintParameters{
                    .dpi = NAN,
                    .min_pixel = 0.f,
                    .max_pixel = (float)npixels.value().width - 1},
                LayoutConstraintParameters{
                    .dpi = NAN,
                    .min_pixel = 0.f,
                    .max_pixel = (float)npixels.value().height - 1},
                render_config,
                scene_graph_config,
                render_results,
                imposter_rsd);
            // // Disable antialiasing to get this to work.
            // VectorialPixels<float, 4> vpx{ArrayShape{size_t(npixels.value().height), size_t(npixels.value().width)}};
            // CHK(glReadPixels(0, 0, npixels.value().width, npixels.value().height, GL_RGBA, GL_FLOAT, vpx->flat_begin()));
            // StbImage4::from_float_rgba(vpx.to_array()).reversed(0).save_to_file("/tmp/imposter-" + debug_prefix_ + ".png");
        }

        rendering_context_.rendering_resources->set_texture(texture_id_, fbs_->texture_color());
        // TODO: Remove StandardRenderLogic
        add_imposter(
            ImposterParameters{
                la.value().sensor_aabb,
                npixels.value().scaled_sensor_aabb,
                float(cam_to_obj2_len)},
            m.t(),
            camera_position(1),
            std::atan2(-cam_to_obj2(0), -cam_to_obj2(1)));
    }
}

float ImposterLogic::near_plane() const {
    return child_logic_.near_plane();
}

float ImposterLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<double, 4, 4>& ImposterLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, double, 3>& ImposterLogic::iv() const {
    return child_logic_.iv();
}

bool ImposterLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void ImposterLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ImposterLogic\n";
    child_logic_.print(ostr, depth + 1);
}
