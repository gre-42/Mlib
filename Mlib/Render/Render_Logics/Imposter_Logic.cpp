#include "Imposter_Logic.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Cameras/Frustum_Camera.hpp>
#include <Mlib/Geometry/Cameras/Frustum_Camera_Config.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera_Config.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At_Aabb.hpp>
#include <Mlib/Geometry/Coordinates/Npixels_For_Dpi.hpp>
#include <Mlib/Geometry/Intersection/Extremal_Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Frustum3.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Imposter_Parameters.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Square_Resource.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Containers/Render_Scene_Thread_Guard.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

OriginalNodeHider::OriginalNodeHider(ImposterLogic& imposter_logic)
: imposter_logic_{imposter_logic}
{}

bool OriginalNodeHider::node_shall_be_hidden(
    const DanglingPtr<const SceneNode>& camera_node,
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
    const DanglingPtr<const SceneNode>& camera_node,
    const ExternalRenderPass& external_render_pass) const
{
    if (external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        return true;
    }
    return false;
}

ImposterLogic::ImposterLogic(
    RenderingResources& rendering_resources,
    RenderLogic& child_logic,
    Scene& scene,
    DanglingRef<SceneNode> orig_node,
    SelectedCameras& cameras,
    const std::string& debug_prefix,
    uint32_t max_texture_size,
    float down_sampling,
    float max_deviation,
    float min_distance,
    uint32_t max_texture_size_deviation,
    float max_dpi_deviation)
    : on_node_clear{ orig_node->on_clear, CURRENT_SOURCE_LOCATION }
    , rendering_resources_{ rendering_resources }
    , child_logic_{ child_logic }
    , scene_{ scene }
    , orig_node_{ orig_node }
    , cameras_{ cameras }
    , old_projected_bbox_{ uninitialized }
    , old_npixels_{ uninitialized }
    , obj_relative_aabb_{ uninitialized }
    , orig_hider{ *this }
    , texture_{ ColormapWithModifiers{
        .filename = VariableAndHash{ "imposter_color" + scene.get_temporary_instance_suffix() },
        .color_mode = ColorMode::RGBA}.compute_hash() }
    , imposter_node_{ nullptr }
    , debug_prefix_{ debug_prefix }
    , max_texture_size_{ max_texture_size }
    , down_sampling_{ down_sampling }
    , max_deviation_{ max_deviation }
    , min_distance_{ min_distance }
    , max_texture_size_deviation_{ max_texture_size_deviation }
    , max_dpi_deviation_{ max_dpi_deviation }
{
    if ((max_texture_size_ < 1) || (max_texture_size_ > 4096)) {
        THROW_OR_ABORT("Imposter texture size out of bounds");
    }
    auto aabb = orig_node_->relative_aabb();
    if (aabb.empty() || aabb.full()) {
        THROW_OR_ABORT("Cannot compute AABB of \"" + debug_prefix_ + '"');
    }
    obj_relative_aabb_ = aabb.data();
    orig_node->insert_node_hider({ orig_hider, CURRENT_SOURCE_LOCATION });
}

ImposterLogic::~ImposterLogic() {
    if (fbs_ != nullptr) {
        // Warning in case of exception during child_logic_.render.
        rendering_resources_.delete_texture(
            texture_,
            DeletionFailureMode::WARN);
    }
    delete_imposter_if_exists();
    orig_node_->remove_node_hider({ orig_hider, CURRENT_SOURCE_LOCATION });
    on_destroy.clear();
}

void ImposterLogic::add_imposter(
    const ImposterParameters& ips,
    const FixedArray<ScenePos, 3>& orig_node_position,
    ScenePos camera_y,
    float angle_y)
{
    assert_true(imposter_node_ == nullptr);
    Material material{
        // .blend_mode = BlendMode::SEMI_CONTINUOUS_08,  // does not work with vegetation
        .blend_mode = BlendMode::BINARY_08,
        .textures_color = { {.texture_descriptor = TextureDescriptor{.color = texture_}} },
        .shading{
            .emissive = OrderableFixedArray<float, 3>{1.f, 1.f, 1.f},
            .ambient = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
            .diffuse = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
            .specular = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f}}};
    material.compute_color_mode();
    Morphology morphology{ .physics_material = PhysicsMaterial::NONE };
    SquareResource res{
        FixedArray<float, 2, 2>::init(ips.pos.min(0), ips.pos.min(1), ips.pos.max(0), ips.pos.max(1)),
        FixedArray<float, 2, 2>::init(ips.uv.min(0), ips.uv.min(1), ips.uv.max(0), ips.uv.max(1)),
        TransformationMatrix<float, float, 3>::identity(),
        material,
        morphology};
    auto new_imposter_node = make_unique_scene_node(
        FixedArray<ScenePos, 3>{orig_node_position(0), camera_y, orig_node_position(2)},
        FixedArray<float, 3>{0.f, angle_y, 0.f},
        1.f,
        PoseInterpolationMode::ENABLED,
        SceneNodeDomain::RENDER);
    res.instantiate_child_renderable(ChildInstantiationOptions{
        .rendering_resources = &rendering_resources_,
        .instance_name = VariableAndHash<std::string>{ "imposter" },
        .scene_node = new_imposter_node.ref(DP_LOC),
        .renderable_resource_filter = RenderableResourceFilter{}});
    new_imposter_node->insert_node_hider({ imposter_hider_, CURRENT_SOURCE_LOCATION });
    scene_.add_root_imposter_node(new_imposter_node.ref(DP_LOC));
    imposter_node_ = std::move(new_imposter_node);
}

void ImposterLogic::delete_imposter_if_exists() {
    if (imposter_node_ != nullptr) {
        RenderSceneThreadGuard rstg{ scene_ };
        scene_.delete_root_imposter_node(imposter_node_.ref(DP_LOC));
        imposter_node_ = nullptr;
        fbs_ = nullptr;
    }
}

std::optional<RenderSetup> ImposterLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void ImposterLogic::render_without_setup(
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
    auto can = cameras_.camera(DP_LOC);
    auto c = can.node->absolute_bijection(frame_id.external_render_pass.time);
    auto m = orig_node_->absolute_model_matrix();
    float dpi;
    {
        auto cam_cp = can.camera->copy();
        cam_cp->set_aspect_ratio(lx.flength() / ly.flength());
        auto vdpi = cam_cp->dpi({ lx.flength(), ly.flength() });
        auto mdpi = mean(vdpi);
        if (any(vdpi - mdpi > max_dpi_deviation_)) {
            lwarn() << "Could not determine unique DPI: " << vdpi;
            return;
        }
        dpi = mdpi / down_sampling_;
        auto mvp = dot2d(cam_cp->projection_matrix().casted<ScenePos>(), (c.view * m).affine());
        VisibilityCheck<ScenePos> vc{mvp};
        if (vc.orthographic()) {
            delete_imposter_if_exists();
            return;
        }
        auto frustum = Frustum3<ScenePos>::from_projection_matrix(mvp);
        if (!frustum.intersects(obj_relative_aabb_)) {
            return;
        }
    }
    auto camera_position = c.model.t;
    auto cam_to_obj2 = FixedArray<ScenePos, 2>{
        m.t(0) - camera_position(0),
        m.t(2) - camera_position(2)};
    auto cam_to_obj2_len2 = sum(squared(cam_to_obj2));
    if (cam_to_obj2_len2 < squared(min_distance_)) {
        delete_imposter_if_exists();
        return;
    }
    auto cam_to_obj2_len = std::sqrt(cam_to_obj2_len2);
    cam_to_obj2 /= cam_to_obj2_len;

    std::optional<GlLookatAabb> la;
    std::optional<CameraSensorAndNPixels> npixels;
    ProjectedBbox projected_bbox = uninitialized;
    bool old_imposter_exists = (imposter_node_ != nullptr);
    bool delete_old_imposter = false;
    bool create_new_imposter = !old_imposter_exists;
    [&](){
        la = gl_lookat_aabb(
            camera_position,
            m,
            obj_relative_aabb_);
        if (!la.has_value()) {
            delete_old_imposter = old_imposter_exists;
            create_new_imposter = false;
            return;
        }

        if (la.has_value()) {
            npixels = npixels_for_dpi(
                la->sensor_aabb,
                dpi,
                1,
                max_texture_size_);
            if (!npixels.has_value()) {
                delete_old_imposter = old_imposter_exists;
                create_new_imposter = false;
                return;    
            }
            if (old_imposter_exists && !delete_old_imposter) {
                if ((std::abs(npixels->width - old_npixels_.width) > max_texture_size_deviation_) ||
                    (std::abs(npixels->height - old_npixels_.height) > max_texture_size_deviation_))
                {
                    delete_old_imposter = old_imposter_exists;
                }
            }
        }
        if (!delete_old_imposter || create_new_imposter) {
            auto iv = TransformationMatrix<float, ScenePos, 3>(
                la->extrinsic_R, camera_position);
            auto mv = (TransformationMatrix<float, ScenePos, 3>::inverse(
                la->extrinsic_R, camera_position) *
                m);
            size_t i = 0;
            if (!obj_relative_aabb_.for_each_corner([&](const FixedArray<ScenePos, 3>& corner){
                auto pc = mv.transform(corner);
                if (pc(2) > -1e-12) {
                    return false;
                }
                auto pc_proj = pc / (-pc(2));
                projected_bbox(i) = iv.transform(pc_proj * cam_to_obj2_len);
                ++i;
                return true;
            }))
            {
                delete_old_imposter = old_imposter_exists;
                create_new_imposter = false;
                return;
            }
        }
        if (old_imposter_exists && !delete_old_imposter) {
            auto mv = c.view * m;
            size_t i = 0;
            if (!obj_relative_aabb_.for_each_corner([&](const FixedArray<ScenePos, 3>& corner){
                auto pc = mv.transform(corner).casted<float>();
                auto pc_old = c.view.transform(old_projected_bbox_(i++)).casted<float>();
                if ((pc(2) > -1e-12) || (pc_old(2) > -1e-12)) {
                    return true;
                }
                auto pc_proj = FixedArray<float, 2>{pc(0), pc(1)} / (-pc(2));
                auto pc_old_proj = FixedArray<float, 2>{pc_old(0), pc_old(1)} / (-pc_old(2));
                if (sum(squared(pc_proj - pc_old_proj)) > squared(max_deviation_ / dpi)) {
                    return false;
                }
                return true;
            }))
            {
                delete_old_imposter = true;
            }
        }
    }();
    if (delete_old_imposter) {
        delete_imposter_if_exists();
    }
    if (create_new_imposter) {
        assert_true(la.has_value());
        auto imposter_camera_node = make_unique_scene_node(
            camera_position,
            matrix_2_tait_bryan_angles(la->extrinsic_R),
            1.f);
        imposter_camera_node->set_camera(
            std::make_unique<FrustumCamera>(
                FrustumCameraConfig::from_sensor_aabb(
                    npixels->scaled_sensor_aabb,
                    la->near_plane,
                    la->far_plane),
                FrustumCamera::Postprocessing::ENABLED));
        RenderedSceneDescriptor imposter_rsd{
            .external_render_pass = {
                ExternalRenderPassType::IMPOSTER_NODE,
                frame_id.external_render_pass.time,
                "",
                orig_node_.ptr(),
                imposter_camera_node.get(DP_LOC)
            },
            .time_id = 0};
        if (fbs_ == nullptr) {
            fbs_ = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        }
        ViewportGuard vg{ npixels->width, npixels->height };
        fbs_->configure({
            .width = npixels->width,
            .height = npixels->height,
            .color_internal_format = GL_RGBA,
            .color_format = GL_RGBA,
            .nsamples_msaa = render_config.imposter_nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{ fbs_ };
            AggregateRendererGuard arg{
                std::make_shared<AggregateArrayRenderer>(rendering_resources_),
                std::make_shared<AggregateArrayRenderer>(rendering_resources_)};
            InstancesRendererGuard irg{
                std::make_shared<ArrayInstancesRenderers>(rendering_resources_),
                std::make_shared<ArrayInstancesRenderer>(rendering_resources_)};
            // notify_rendering(CURRENT_SOURCE_LOCATION);
            // CHK(glClearColor(1.f, 0.f, 1.f, 1.f));
            // CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            child_logic_.render_toplevel(
                LayoutConstraintParameters{
                    .dpi = NAN,
                    .min_pixel = 0.f,
                    .end_pixel = (float)npixels->width},
                LayoutConstraintParameters{
                    .dpi = NAN,
                    .min_pixel = 0.f,
                    .end_pixel = (float)npixels->height},
                render_config,
                scene_graph_config,
                render_results,
                imposter_rsd);
            // // Disable antialiasing to get this to work.
            // VectorialPixels<float, 4> vpx{ArrayShape{size_t(npixels->height), size_t(npixels->width)}};
            // CHK(glReadPixels(0, 0, npixels->width, npixels->height, GL_RGBA, GL_FLOAT, vpx->flat_begin()));
            // StbImage4::from_float_rgba(vpx.to_array()).reversed(0).save_to_file("/tmp/imposter-" + debug_prefix_ + ".png");
        }

        rendering_resources_.set_texture(texture_, fbs_->texture_color());
        add_imposter(
            ImposterParameters{
                la->sensor_aabb,
                npixels->scaled_sensor_aabb,
                float(cam_to_obj2_len)},
            m.t,
            camera_position(1),
            (float)std::atan2(-cam_to_obj2(0), -cam_to_obj2(1)));
        old_npixels_ = *npixels;
        old_projected_bbox_ = projected_bbox;
    }
}

void ImposterLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ImposterLogic\n";
    child_logic_.print(ostr, depth + 1);
}
