#include "Lightmap_Logic.hpp"
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Optional.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

LightmapLogic::LightmapLogic(
    RenderLogic& child_logic,
    ExternalRenderPassType render_pass_type,
    DanglingRef<SceneNode> light_node,
    std::string resource_suffix,
    std::string black_node_name,
    bool with_depth_texture,
    int lightmap_width,
    int lightmap_height)
: child_logic_{child_logic},
  rendering_context_{RenderingContextStack::resource_context()},
  render_pass_type_{render_pass_type},
  light_node_{light_node},
  resource_suffix_{std::move(resource_suffix)},
  black_node_name_{std::move(black_node_name)},
  with_depth_texture_{with_depth_texture},
  lightmap_width_{lightmap_width},
  lightmap_height_{lightmap_height},
  deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{
    if (!any(render_pass_type & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        THROW_OR_ABORT("LightmapLogic::LightmapLogic: unknown lightmap render pass type");
    }
}

LightmapLogic::~LightmapLogic() {
    if (fbs_ != nullptr) {
        // Warning in case of exception during child_logic_.render.
        rendering_context_.rendering_resources->delete_texture("lightmap_color." + resource_suffix_, DeletionFailureMode::WARN);
        rendering_context_.rendering_resources->delete_vp("lightmap_color." + resource_suffix_, DeletionFailureMode::WARN);
        if (with_depth_texture_) {
            rendering_context_.rendering_resources->delete_texture("lightmap_depth." + resource_suffix_, DeletionFailureMode::WARN);
            rendering_context_.rendering_resources->delete_vp("lightmap_depth." + resource_suffix_, DeletionFailureMode::WARN);
        }
    }
}

void LightmapLogic::deallocate() {
    fbs_ = nullptr;
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
        ViewportGuard vg{lightmap_width_, lightmap_height_};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        RenderedSceneDescriptor light_rsd{
            .external_render_pass = {render_pass_type_, black_node_name_, nullptr, light_node_.ptr()},
            .time_id = 0,
            .light_resource_suffix = resource_suffix_};
#pragma GCC diagnostic pop
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
            RenderToFrameBufferGuard rfg{*fbs_};
            RenderingContextGuard rrg{rendering_context_};
            // Non-static lights are not aggregated at all due to the following lines
            // in Scene::render:
            //   bool is_foreground_task = any(external_render_pass.pass & ExternalRenderPassType::IS_STATIC_MASK);
            //   bool is_background_task = (external_render_pass.pass == ExternalRenderPassType::STANDARD);
            bool create_render_guards = any(light_rsd.external_render_pass.pass & ExternalRenderPassType::IS_STATIC_MASK);
            Optional<AggregateRendererGuard> arg{
                create_render_guards ? OptionalState::SOME : OptionalState::NONE,
                std::make_shared<AggregateArrayRenderer>(),
                std::make_shared<AggregateArrayRenderer>()};
            Optional<InstancesRendererGuard> irg{
                create_render_guards ? OptionalState::SOME : OptionalState::NONE,
                std::make_shared<ArrayInstancesRenderers>(),
                std::make_shared<ArrayInstancesRenderer>()};
            child_logic_.render(
                LayoutConstraintParameters{
                    .dpi = NAN,
                    .min_pixel = 0.f,
                    .max_pixel = (float)lightmap_width_ - 1.f},
                LayoutConstraintParameters{
                    .dpi = NAN,
                    .min_pixel = 0.f,
                    .max_pixel = (float)lightmap_width_ - 1.f},
                render_config,
                scene_graph_config,
                render_results, light_rsd);
            // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
            // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
            // StbImage3::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.png");
        }

        rendering_context_.rendering_resources->set_texture("lightmap_color." + resource_suffix_, fbs_->texture_color());
        rendering_context_.rendering_resources->set_vp("lightmap_color." + resource_suffix_, vp());
        if (with_depth_texture_) {
            rendering_context_.rendering_resources->set_texture("lightmap_depth." + resource_suffix_, fbs_->texture_depth());
            rendering_context_.rendering_resources->set_vp("lightmap_depth." + resource_suffix_, vp());
        }
    }
}

float LightmapLogic::near_plane() const {
    return child_logic_.near_plane();
}

float LightmapLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<double, 4, 4>& LightmapLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, double, 3>& LightmapLogic::iv() const {
    return child_logic_.iv();
}

bool LightmapLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void LightmapLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "LightmapLogic\n";
    child_logic_.print(ostr, depth + 1);
}
