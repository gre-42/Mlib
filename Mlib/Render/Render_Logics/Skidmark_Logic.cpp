#include "Skidmark_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
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
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/Particle_Substrate.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SkidmarkLogic::SkidmarkLogic(
    DanglingRef<SceneNode> skidmark_node,
    std::shared_ptr<Skidmark> skidmark,
    IParticleRenderer& particle_renderer,
    int texture_width,
    int texture_height)
    : on_skidmark_node_clear{ skidmark_node->on_clear, CURRENT_SOURCE_LOCATION }
    , fbs_{ uninitialized }
    , skidmark_node_{ skidmark_node }
    , skidmark_{ skidmark }
    , particle_renderer_{ particle_renderer }
    , texture_width_{ texture_width }
    , texture_height_{ texture_height }
    , old_fbs_id_{ 0 }
    , old_camera_position_{ fixed_nans<ScenePos, 3>() }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{}

SkidmarkLogic::~SkidmarkLogic() {
    on_destroy.clear();
    deallocate();
}

void SkidmarkLogic::deallocate() {
    // skidmark_->texture = nullptr;
    fbs_(0) = nullptr;
    fbs_(1) = nullptr;
}

std::optional<RenderSetup> SkidmarkLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void SkidmarkLogic::render_without_setup(
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
    auto camera = skidmark_node_->get_camera(CURRENT_SOURCE_LOCATION);
    const auto* ortho_camera = dynamic_cast<OrthoCamera*>(&camera.get());
    if (ortho_camera == nullptr) {
        THROW_OR_ABORT("Skidmark camera is not an ortho-camera");
    }
    auto p = ortho_camera->projection_matrix();
    auto bi = skidmark_node_->absolute_bijection(std::chrono::steady_clock::time_point());
    auto vp = dot2d(p.casted<ScenePos>(), bi.view.affine());
    fbs_(new_fbs_id)->configure({
        .width = texture_width_,
        .height = texture_height_,
        .depth_kind = FrameBufferChannelKind::NONE,
        .nsamples_msaa = 1});
    {
        if (fbs_(old_fbs_id_) != nullptr) {
            if (old_render_texture_logic_ == nullptr) {
                old_render_texture_logic_ = std::make_shared<FillWithTextureLogic>(
                    fbs_(old_fbs_id_)->texture_color(),
                    CullFaceMode::NO_CULL);
            } else {
                old_render_texture_logic_->set_image_resource_name(
                    fbs_(old_fbs_id_)->texture_color());
            }
        }
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>> lights;
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>> skidmarks;
        RenderToFrameBufferGuard rfg{ fbs_(new_fbs_id) };
        {
            ViewportGuard vg{ texture_width_, texture_height_ };
            clear_color({ 1.f, 1.f, 1.f, 1.f });
        }
        if (fbs_(old_fbs_id_) != nullptr) {
            auto dpi = ortho_camera->dpi({
                (float)texture_width_,
                (float)texture_height_});
            auto diff = bi.view.rotate((old_camera_position_ - bi.model.t).casted<float>());
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
                bi.model,
                lights,
                skidmarks,
                scene_graph_config,
                render_config,
                { ExternalRenderPassType::STANDARD });
        }
        old_fbs_id_ = new_fbs_id;
        old_camera_position_ = bi.model.t;
        // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
        // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
        // StbImage3::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.png");
    }
    skidmark_->texture = fbs_(new_fbs_id)->texture_color();
    skidmark_->vp = vp;
}

void SkidmarkLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SkidmarkLogic\n";
}
