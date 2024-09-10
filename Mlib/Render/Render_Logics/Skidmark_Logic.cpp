#include "Skidmark_Logic.hpp"
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/Particle_Substrate.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SkidmarkLogic::SkidmarkLogic(
    RenderingResources& rendering_resources,
    DanglingRef<SceneNode> skidmark_node,
    std::string resource_suffix,
    IParticleRenderer& particle_renderer,
    int texture_width,
    int texture_height)
    : on_skidmark_node_clear{ skidmark_node->on_clear, CURRENT_SOURCE_LOCATION }
    , rendering_resources_{ rendering_resources }
    , fbs_{ uninitialized }
    , skidmark_node_{ skidmark_node }
    , resource_suffix_{ std::move(resource_suffix) }
    , particle_renderer_{ particle_renderer }
    , texture_width_{ texture_width }
    , texture_height_{ texture_height }
    , old_fbs_id_{ 0 }
    , old_camera_position_{ fixed_nans<ScenePos, 3>() }
    , colormap_{ .filename = VariableAndHash{ "skidmark." + resource_suffix_ }, .color_mode = ColorMode::RGB }
    , vp_{ "skidmark." + resource_suffix_ }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{
    colormap_.compute_hash();
}

SkidmarkLogic::~SkidmarkLogic() {
    on_destroy.clear();
    deallocate();
}

void SkidmarkLogic::deallocate() {
    auto null = std::unique_ptr<FrameBuffer>{ nullptr };
    if (any(fbs_ != null)) {
        // Warning in case of exception during child_logic_.render.
        rendering_resources_.delete_texture(colormap_, DeletionFailureMode::WARN);
        rendering_resources_.delete_vp(vp_, DeletionFailureMode::WARN);
        fbs_(0) = nullptr;
        fbs_(1) = nullptr;
    }
}

void SkidmarkLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("SkidmarkLogic::render");
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        THROW_OR_ABORT("SkidmarkLogic received wrong rendering");
    }
    size_t new_fbs_id = 1 - old_fbs_id_;
    if (fbs_(new_fbs_id) == nullptr) {
        fbs_(new_fbs_id) = std::make_unique<FrameBuffer>(CURRENT_SOURCE_LOCATION);
    }
    const auto* skidmark_camera = dynamic_cast<OrthoCamera*>(&skidmark_node_->get_camera());
    if (skidmark_camera == nullptr) {
        THROW_OR_ABORT("Skidmark camera is not an ortho-camera");
    }
    auto p = skidmark_camera->projection_matrix();
    auto v = skidmark_node_->absolute_view_matrix();
    auto iv = skidmark_node_->absolute_model_matrix();
    auto vp = dot2d(p.casted<ScenePos>(), v.affine());
    fbs_(new_fbs_id)->configure({
        .width = texture_width_,
        .height = texture_height_,
        .depth_kind = FrameBufferChannelKind::NONE,
        .nsamples_msaa = 1});
    {
        if (old_render_texture_logic_ == nullptr) {
            old_render_texture_logic_ = std::make_shared<FillWithTextureLogic>(
                rendering_resources_,
                colormap_,
                ResourceUpdateCycle::ALWAYS,
                CullFaceMode::NO_CULL);
        } else if (fbs_(old_fbs_id_) != nullptr) {
            old_render_texture_logic_->update_texture_id();
        }
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, Light*>> lights;
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, Skidmark*>> skidmarks;
        RenderToFrameBufferGuard rfg{ *fbs_(new_fbs_id) };
        RenderToScreenGuard rsg{ CURRENT_SOURCE_LOCATION };
        {
            ViewportGuard vg{ texture_width_, texture_height_ };
            clear_color({ 1.f, 1.f, 1.f, 1.f });
        }
        if (fbs_(old_fbs_id_) != nullptr) {
            auto dpi = skidmark_camera->dpi(
                (float)texture_width_,
                (float)texture_height_);
            auto diff = v.rotate((old_camera_position_ - iv.t()).casted<float>());
            ViewportGuard vg{
                diff(0) * dpi(0),
                diff(1) * dpi(1),
                (float)texture_width_,
                (float)texture_height_ };
            old_render_texture_logic_->render_wo_update_and_bind();
        }
        {
            ViewportGuard vg{ texture_width_, texture_height_ };
            RenderConfigGuard rcg{ render_config, ExternalRenderPassType::STANDARD };
            particle_renderer_.render(
                ParticleSubstrate::SKIDMARK,
                vp,
                iv,
                lights,
                skidmarks,
                scene_graph_config,
                render_config,
                { ExternalRenderPassType::STANDARD });
        }
        old_fbs_id_ = new_fbs_id;
        old_camera_position_ = iv.t();
        // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
        // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
        // StbImage3::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.png");
    }
    rendering_resources_.set_texture(
        colormap_,
        fbs_(new_fbs_id)->texture_color(),
        ResourceOwner::CALLER);
    rendering_resources_.set_vp(vp_, vp);
}

void SkidmarkLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SkidmarkLogic\n";
}
