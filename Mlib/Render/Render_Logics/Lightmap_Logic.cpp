#include "Lightmap_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

LightmapLogic::LightmapLogic(
    RenderingResources& rendering_resources,
    RenderLogic& child_logic,
    ExternalRenderPassType render_pass_type,
    DanglingRef<SceneNode> light_node,
    std::shared_ptr<Light> light,
    VariableAndHash<std::string> black_node_name,
    bool with_depth_texture,
    int lightmap_width,
    int lightmap_height,
    const FixedArray<uint32_t, 2>& smooth_niterations)
    : on_child_logic_destroy{ child_logic.on_destroy, CURRENT_SOURCE_LOCATION }
    , on_node_clear{ light_node->on_clear, CURRENT_SOURCE_LOCATION }
    , rendering_resources_{ rendering_resources }
    , child_logic_{ child_logic }
    , fbs_{
        std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION),
        std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION) }
    , render_pass_type_{ render_pass_type }
    , light_node_{ light_node }
    , black_node_name_{ std::move(black_node_name) }
    , light_{ light }
    , with_depth_texture_{ with_depth_texture }
    , lightmap_width_{ lightmap_width }
    , lightmap_height_{ lightmap_height }
    , smooth_niterations_{ smooth_niterations }
{
    if (!any(render_pass_type & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        THROW_OR_ABORT("LightmapLogic::LightmapLogic: unknown lightmap render pass type");
    }
    if (with_depth_texture_ && any(smooth_niterations_ != 0u)) {
        THROW_OR_ABORT("LightmapLogic::LightmapLogic: depth textures do not support smoothing");
    }
}

LightmapLogic::~LightmapLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> LightmapLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void LightmapLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("LightmapLogic::render");
    if (!any(frame_id.external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK)) {
        THROW_OR_ABORT("LightmapLogic received wrong rendering");
    }
    if ((!fbs_[0]->is_configured() && !fbs_[1]->is_configured()) || any(render_pass_type_ & ExternalRenderPassType::LIGHTMAP_IS_DYNAMIC_MASK)) {
        ViewportGuard vg{ lightmap_width_, lightmap_height_ };
        RenderedSceneDescriptor light_rsd{
            .external_render_pass = {frame_id.external_render_pass.user_id, render_pass_type_, frame_id.external_render_pass.time, black_node_name_, nullptr, light_node_.ptr()},
            .time_id = 0};
        size_t target_id = 0;
        auto border_color = (float)!any(render_pass_type_ & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK);
        fbs_[target_id]->configure({
            .width = lightmap_width_,
            .height = lightmap_height_,
            .color_magnifying_interpolation_mode = InterpolationMode::LINEAR,
            .depth_kind = with_depth_texture_
                ? FrameBufferChannelKind::TEXTURE
                : FrameBufferChannelKind::ATTACHMENT,
            .wrap_s = GL_CLAMP_TO_BORDER,
            .wrap_t = GL_CLAMP_TO_BORDER,
            .border_color = make_orderable(fixed_full<float, 4>(border_color)),
            .nsamples_msaa = render_config.lightmap_nsamples_msaa});
        if (!with_depth_texture_) {
            fbs_[1 - target_id]->configure({
                .width = lightmap_width_,
                .height = lightmap_height_,
                .color_magnifying_interpolation_mode = InterpolationMode::LINEAR,
                .depth_kind = FrameBufferChannelKind::NONE,
                .wrap_s = GL_CLAMP_TO_BORDER,
                .wrap_t = GL_CLAMP_TO_BORDER,
                .border_color = make_orderable(fixed_full<float, 4>(border_color))});
        }
        std::optional<RenderSetup> setup;
        {
            RenderToFrameBufferGuard rfg{ fbs_[target_id] };
            // Non-static lights are not aggregated at all due to the following lines
            // in Scene::render:
            //   bool is_foreground_task = any(external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK);
            //   bool is_background_task = any(external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK);
            bool create_render_guards = any(light_rsd.external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK);
            std::optional<AggregateRendererGuard> arg;
            std::optional<InstancesRendererGuard> irg;
            if (create_render_guards) {
                arg.emplace(
                    nullptr,
                    nullptr,
                    std::make_shared<AggregateArrayRenderer>(rendering_resources_),
                    std::make_shared<AggregateArrayRenderer>(rendering_resources_));
                irg.emplace(
                    nullptr,
                    nullptr,
                    std::make_shared<ArrayInstancesRenderers>(rendering_resources_),
                    std::make_shared<ArrayInstancesRenderer>(rendering_resources_));
            }
            auto lx = LayoutConstraintParameters{
                .dpi = NAN,
                .min_pixel = 0.f,
                .end_pixel = (float)lightmap_width_};
            auto ly = LayoutConstraintParameters{
                .dpi = NAN,
                .min_pixel = 0.f,
                .end_pixel = (float)lightmap_height_};
            setup.emplace(child_logic_.render_setup(lx, ly, light_rsd));
            if (!setup.has_value()) {
                THROW_OR_ABORT("LightmapLogic::render could not determine child setup");
            }
            child_logic_.render_with_setup(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                light_rsd,
                *setup);
            // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
            // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
            // StbImage3::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.png");
        }
        if (!with_depth_texture_ && any(smooth_niterations_ != 0u)) {
            lowpass_.render(lightmap_width_, lightmap_height_, smooth_niterations_, fbs_, target_id);
        }

        light_->lightmap_color = fbs_[target_id]->texture_color();
        light_->lightmap_depth = fbs_[target_id]->texture_depth();
        light_->vp = setup->vp;
    }
}

void LightmapLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "LightmapLogic\n";
    child_logic_.print(ostr, depth + 1);
}
