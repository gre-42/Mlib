#include "Lightmap_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <optional>

using namespace Mlib;

LightmapLogic::LightmapLogic(
    RenderLogic& child_logic,
    ExternalRenderPassType render_pass_type,
    const std::string& light_node_name,
    const std::string& black_node_name,
    bool with_depth_texture)
: child_logic_{child_logic},
  rendering_context_{RenderingContextStack::resource_context()},
  render_pass_type_{render_pass_type},
  light_node_name_{light_node_name},
  black_node_name_{black_node_name},
  with_depth_texture_{with_depth_texture}
{
    if (!bool(render_pass_type & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        throw std::runtime_error("LightmapLogic::LightmapLogic: unknown lightmap render pass type");
    }
}

LightmapLogic::~LightmapLogic() {
    if (fbs_ != nullptr) {
        // Warning in case of exception during child_logic_.render.
        rendering_context_.rendering_resources->delete_texture("lightmap_color." + light_node_name_, DeletionFailureMode::WARN);
        rendering_context_.rendering_resources->delete_vp("lightmap_color." + light_node_name_, DeletionFailureMode::WARN);
        if (with_depth_texture_) {
            rendering_context_.rendering_resources->delete_texture("lightmap_depth." + light_node_name_, DeletionFailureMode::WARN);
            rendering_context_.rendering_resources->delete_vp("lightmap_depth." + light_node_name_, DeletionFailureMode::WARN);
        }
    }
}

void LightmapLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("LightmapLogic::render");
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        throw std::runtime_error("LightmapLogic received wrong rendering");
    }
    if ((fbs_ == nullptr) || bool(render_pass_type_ & ExternalRenderPassType::LIGHTMAP_IS_DYNAMIC_MASK)) {
        GLsizei lightmap_width = black_node_name_.empty()
            ? render_config.scene_lightmap_width
            : render_config.black_lightmap_width;
        GLsizei lightmap_height = black_node_name_.empty()
            ? render_config.scene_lightmap_height
            : render_config.black_lightmap_height;
        ViewportGuard vg{0, 0, lightmap_width, lightmap_height};
        RenderedSceneDescriptor light_rsd{.external_render_pass = {render_pass_type_, black_node_name_}, .time_id = 0, .light_node_name = light_node_name_};
        if (fbs_ == nullptr) {
            fbs_ = std::make_unique<FrameBufferMsaa>();
        }
        fbs_->configure({
            .width = lightmap_width,
            .height = lightmap_height,
            .with_depth_texture = with_depth_texture_,
            .nsamples_msaa = render_config.lightmap_nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{*fbs_};
            RenderingContextGuard rrg{rendering_context_};
            bool create_render_guards = bool(light_rsd.external_render_pass.pass & ExternalRenderPassType::IS_STATIC_MASK);
            auto arg = create_render_guards
                ? std::make_optional<AggregateRendererGuard>(
                    std::make_shared<AggregateArrayRenderer>(),
                    std::make_shared<AggregateArrayRenderer>())
                : std::nullopt;
            auto irg = create_render_guards
                ? std::make_optional<InstancesRendererGuard>(
                    std::make_shared<ArrayInstancesRenderers>(),
                    std::make_shared<ArrayInstancesRenderer>())
                : std::nullopt;
            child_logic_.render(lightmap_width, lightmap_height, render_config, scene_graph_config, render_results, light_rsd);
            // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
            // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
            // PpmImage::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.ppm");
        }

        rendering_context_.rendering_resources->set_texture("lightmap_color." + light_node_name_, fbs_->fb.texture_color);
        rendering_context_.rendering_resources->set_vp("lightmap_color." + light_node_name_, vp());
        if (with_depth_texture_) {
            rendering_context_.rendering_resources->set_texture("lightmap_depth." + light_node_name_, fbs_->fb.texture_depth);
            rendering_context_.rendering_resources->set_vp("lightmap_depth." + light_node_name_, vp());
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
