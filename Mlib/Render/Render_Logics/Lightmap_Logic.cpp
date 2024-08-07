#include "Lightmap_Logic.hpp"
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

LightmapLogic::LightmapLogic(
    RenderingResources& rendering_resources,
    RenderLogic& child_logic,
    ExternalRenderPassType render_pass_type,
    DanglingRef<SceneNode> light_node,
    ColormapWithModifiers colormap_color,
    ColormapWithModifiers colormap_depth,
    std::string black_node_name,
    bool with_depth_texture,
    int lightmap_width,
    int lightmap_height)
    : on_child_logic_destroy{ child_logic.on_destroy, CURRENT_SOURCE_LOCATION }
    , on_node_clear{ light_node->on_clear, CURRENT_SOURCE_LOCATION }
    , rendering_resources_{ rendering_resources }
    , child_logic_{ child_logic }
    , render_pass_type_{ render_pass_type }
    , light_node_{ light_node }
    , black_node_name_{ std::move(black_node_name) }
    , with_depth_texture_{ with_depth_texture }
    , lightmap_width_{ lightmap_width }
    , lightmap_height_{ lightmap_height }
    , colormap_color_{ std::move(colormap_color) }
    , colormap_depth_{ std::move(colormap_depth) }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{
    if (!any(render_pass_type & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        THROW_OR_ABORT("LightmapLogic::LightmapLogic: unknown lightmap render pass type");
    }
}

LightmapLogic::~LightmapLogic() {
    deallocate();
    on_destroy.clear();
}

void LightmapLogic::deallocate() {
    if (fbs_ != nullptr) {
        // Warning in case of exception during child_logic_.render.
        rendering_resources_.delete_texture(colormap_color_, DeletionFailureMode::WARN);
        rendering_resources_.delete_vp(colormap_color_.filename, DeletionFailureMode::WARN);
        if (with_depth_texture_) {
            rendering_resources_.delete_texture(colormap_depth_, DeletionFailureMode::WARN);
            rendering_resources_.delete_vp(colormap_depth_.filename, DeletionFailureMode::WARN);
        }
        fbs_ = nullptr;
    }
}

void LightmapLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("LightmapLogic::render");
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        THROW_OR_ABORT("LightmapLogic received wrong rendering");
    }
    if ((fbs_ == nullptr) || any(render_pass_type_ & ExternalRenderPassType::LIGHTMAP_IS_DYNAMIC_MASK)) {
        ViewportGuard vg{ lightmap_width_, lightmap_height_ };
        RenderedSceneDescriptor light_rsd{
            .external_render_pass = {render_pass_type_, frame_id.external_render_pass.time, black_node_name_, nullptr, light_node_.ptr()},
            .time_id = 0};
        if (fbs_ == nullptr) {
            fbs_ = std::make_unique<FrameBuffer>();
        }
        fbs_->configure({
            .width = lightmap_width_,
            .height = lightmap_height_,
            .depth_kind = with_depth_texture_
                ? FrameBufferChannelKind::TEXTURE
                : FrameBufferChannelKind::ATTACHMENT,
            .nsamples_msaa = render_config.lightmap_nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{ *fbs_ };
            // Non-static lights are not aggregated at all due to the following lines
            // in Scene::render:
            //   bool is_foreground_task = any(external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK);
            //   bool is_background_task = (external_render_pass.pass == ExternalRenderPassType::STANDARD);
            bool create_render_guards = any(light_rsd.external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK);
            std::optional<AggregateRendererGuard> arg;
            std::optional<InstancesRendererGuard> irg;
            if (create_render_guards) {
                arg.emplace(
                    std::make_shared<AggregateArrayRenderer>(rendering_resources_),
                    std::make_shared<AggregateArrayRenderer>(rendering_resources_));
                irg.emplace(
                    std::make_shared<ArrayInstancesRenderers>(rendering_resources_),
                    std::make_shared<ArrayInstancesRenderer>(rendering_resources_));
            }
            child_logic_.render(
                LayoutConstraintParameters{
                    .dpi = NAN,
                    .min_pixel = 0.f,
                    .end_pixel = (float)lightmap_width_},
                LayoutConstraintParameters{
                    .dpi = NAN,
                    .min_pixel = 0.f,
                    .end_pixel = (float)lightmap_height_},
                render_config,
                scene_graph_config,
                render_results,
                light_rsd);
            // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
            // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
            // StbImage3::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.png");
        }

        rendering_resources_.set_texture(colormap_color_, fbs_->texture_color(), ResourceOwner::CALLER);
        rendering_resources_.set_vp(colormap_color_.filename, vp());
        if (with_depth_texture_) {
            rendering_resources_.set_texture(colormap_depth_, fbs_->texture_depth(), ResourceOwner::CALLER);
            rendering_resources_.set_vp(colormap_depth_.filename, vp());
        }
    }
}

float LightmapLogic::near_plane() const {
    return child_logic_.near_plane();
}

float LightmapLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<ScenePos, 4, 4>& LightmapLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, ScenePos, 3>& LightmapLogic::iv() const {
    return child_logic_.iv();
}

bool LightmapLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void LightmapLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "LightmapLogic\n";
    child_logic_.print(ostr, depth + 1);
}
