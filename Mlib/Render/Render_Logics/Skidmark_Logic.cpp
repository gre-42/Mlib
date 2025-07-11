#include "Skidmark_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

SkidmarkLogic::SkidmarkLogic(
    DanglingRef<SceneNode> skidmark_node,
    std::shared_ptr<Skidmark> skidmark,
    IParticleRenderer& particle_renderer,
    int texture_width,
    int texture_height)
    : MovingNodeLogic{ skidmark_node }
    , on_skidmark_node_clear{ skidmark_node->on_clear, CURRENT_SOURCE_LOCATION }
    , fbs_{ uninitialized }
    , skidmark_{ std::move(skidmark) }
    , particle_renderer_{ particle_renderer }
    , texture_width_{ texture_width }
    , texture_height_{ texture_height }
    , old_fbs_id_{ 0 }
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

void SkidmarkLogic::render_moving_node(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const Bijection<TransformationMatrix<float, ScenePos, 3>>& bi,
    const FixedArray<ScenePos, 4, 4>& vp,
    const std::optional<FixedArray<float, 2>>& offset)
{
    LOG_FUNCTION("SkidmarkLogic::render");
    if (!any(frame_id.external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK)) {
        THROW_OR_ABORT("SkidmarkLogic received wrong rendering");
    }
    size_t new_fbs_id = 1 - old_fbs_id_;
    if (fbs_(new_fbs_id) == nullptr) {
        fbs_(new_fbs_id) = std::make_unique<FrameBuffer>(CURRENT_SOURCE_LOCATION);
    }
    auto border_color = [&]() -> OrderableFixedArray<float, 4> {
        switch (skidmark_->particle_type) {
        case ParticleType::NONE:
            THROW_OR_ABORT("Particle type \"none\" does not require a skidmark logic");
        case ParticleType::SMOKE:
            THROW_OR_ABORT("Smoke does not require a skidmark logic");
        case ParticleType::SKIDMARK:
            return { 1.f, 1.f, 1.f, 1.f };
        case ParticleType::WATER_WAVE:
            THROW_OR_ABORT("Water wave does not require a skidmark logic");
        case ParticleType::SEA_SPRAY:
            return { 0.f, 0.f, 0.f, 1.f };
        }
        THROW_OR_ABORT("Unknown particle type");
    }();
    fbs_(new_fbs_id)->configure({
        .width = texture_width_,
        .height = texture_height_,
        .color_magnifying_interpolation_mode = InterpolationMode::LINEAR,
        .depth_kind = FrameBufferChannelKind::NONE,
        .wrap_s = GL_CLAMP_TO_BORDER,
        .wrap_t = GL_CLAMP_TO_BORDER,
        .border_color = border_color,
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
            clear_color(border_color);
        }
        if (fbs_(old_fbs_id_) != nullptr) {
            assert_true(offset.has_value());
            ViewportGuard vg{
                (float)texture_width_ * (*offset)(0),
                (float)texture_height_ * (*offset)(1),
                (float)texture_width_,
                (float)texture_height_ };
            old_render_texture_logic_->render_wo_update_and_bind();
        }
        {
            ViewportGuard vg{ texture_width_, texture_height_ };
            RenderConfigGuard rcg{ render_config, ExternalRenderPassType::STANDARD };
            particle_renderer_.render(
                vp,
                bi.model,
                bi.view,
                nullptr,    // dynamic_style
                lights,
                skidmarks,
                scene_graph_config,
                render_config,
                { frame_id, InternalRenderPass::PARTICLES },
                nullptr,    // animation_state
                nullptr);   // color_style
        }
        old_fbs_id_ = new_fbs_id;
        // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
        // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
        // StbImage3::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.png");
    }
    skidmark_->texture = fbs_(new_fbs_id)->texture_color();
    skidmark_->vp = vp;
}

void SkidmarkLogic::save_debug_image(const std::string& filename) const {
    auto fb = fbs_(old_fbs_id_);
    if (fb == nullptr) {
        THROW_OR_ABORT("SkidmarkLogic::save_debug_image: Not yet rendered");
    }
    StbImage3(fb->color_to_stb_image(3)).save_to_file(filename);
}

void SkidmarkLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SkidmarkLogic";
}
