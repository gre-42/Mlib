#include "Render_To_Texture_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

RenderToTextureLogic::RenderToTextureLogic(
    RenderLogic& child_logic,
    RenderingResources& rendering_resources,
    ResourceUpdateCycle update_cycle,
    FrameBufferChannelKind depth_kind,
    VariableAndHash<std::string> color_texture_name,
    VariableAndHash<std::string> depth_texture_name,
    const FixedArray<int, 2>& texture_size,
    FocusFilter focus_filter)
    : child_logic_{ child_logic }
    , rendering_resources_{ rendering_resources }
    , update_cycle_{ update_cycle }
    , depth_kind_{ depth_kind }
    , color_texture_name_{ std::move(color_texture_name) }
    , depth_texture_name_{ std::move(depth_texture_name) }
    , texture_size_{ texture_size }
    , focus_filter_{ std::move(focus_filter) }
{}

RenderToTextureLogic::~RenderToTextureLogic() {
    if (fbs_ != nullptr) {
        // Warning in case of exception during child_logic_.render.
        rendering_resources_.delete_texture({ .filename = color_texture_name_, .color_mode = ColorMode::RGBA }, DeletionFailureMode::WARN);
        if (depth_kind_ == FrameBufferChannelKind::TEXTURE) {
            rendering_resources_.delete_texture({ .filename = depth_texture_name_, .color_mode = ColorMode::GRAYSCALE }, DeletionFailureMode::WARN);
        }
    }
}

std::optional<RenderSetup> RenderToTextureLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.try_render_setup(lx, ly, frame_id);
}

bool RenderToTextureLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("RenderToTextureLogic::render");
    if ((fbs_ == nullptr) || (update_cycle_ == ResourceUpdateCycle::ALWAYS)) {
        ViewportGuard vg{ texture_size_(0), texture_size_(1) };
        if (fbs_ == nullptr) {
            fbs_ = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        }
        fbs_->configure({
            .width = texture_size_(0),
            .height = texture_size_(1),
            .color_filter_type = GL_NEAREST,
            .depth_kind = depth_kind_,
            .nsamples_msaa = render_config.nsamples_msaa});
        {
            RenderToFrameBufferGuard fbg{ fbs_ };
            child_logic_.render_auto_setup(
                LayoutConstraintParameters{
                    .dpi = lx.dpi,
                    .min_pixel = 0.f,
                    .end_pixel = (float)texture_size_(0)},
                LayoutConstraintParameters{
                    .dpi = ly.dpi,
                    .min_pixel = 0.f,
                    .end_pixel = (float)texture_size_(1)},
                render_config,
                scene_graph_config,
                render_results,
                frame_id,
                setup);
            // VectorialPixels<float, 3> vpx{ArrayShape{size_t(lightmap_width), size_t(lightmap_height)}};
            // CHK(glReadPixels(0, 0, lightmap_width, lightmap_height, GL_RGB, GL_FLOAT, vpx->flat_iterable().begin()));
            // PpmImage::from_float_rgb(vpx.to_array()).save_to_file("/tmp/lightmap.ppm");
        }
        rendering_resources_.set_texture({ .filename = color_texture_name_, .color_mode = ColorMode::RGBA }, fbs_->texture_color());
        if (depth_kind_ == FrameBufferChannelKind::TEXTURE) {
            rendering_resources_.set_texture({ .filename = depth_texture_name_, .color_mode = ColorMode::GRAYSCALE }, fbs_->texture_depth());
        }
    }
    return true;
}

FocusFilter RenderToTextureLogic::focus_filter() const {
    return focus_filter_;
}

void RenderToTextureLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RenderToTextureLogic\n";
    child_logic_.print(ostr, depth + 1);
}
