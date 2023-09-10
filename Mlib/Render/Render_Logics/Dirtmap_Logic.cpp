#include "Dirtmap_Logic.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DirtmapLogic::DirtmapLogic(
    RenderLogic& child_logic)
: child_logic_{child_logic},
  rendering_context_{RenderingContextStack::resource_context()},
  generated_{false}
{}

DirtmapLogic::~DirtmapLogic() = default;

void DirtmapLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("DirtmapLogic::render");
    if (frame_id.external_render_pass.pass == ExternalRenderPassType::DIRTMAP) {
        THROW_OR_ABORT("DirtmapLogic received dirtmap rendering");
    }
    if (filename_.empty()) {
        return;
    }
    if (!generated_) {
        // Calculate camera position
        {
            RenderingContextGuard rrg{rendering_context_};
            AggregateRendererGuard arg{
                std::make_shared<AggregateArrayRenderer>(),
                std::make_shared<AggregateArrayRenderer>()};
            InstancesRendererGuard irg{
                std::make_shared<ArrayInstancesRenderers>(),
                std::make_shared<ArrayInstancesRenderer>()};
            child_logic_.render(
                LayoutConstraintParameters{
                    .dpi = 96.f,
                    .min_pixel = 0.f,
                    .max_pixel = 639.f},
                LayoutConstraintParameters{
                    .dpi = 96.f,
                    .min_pixel = 0.f,
                    .max_pixel = 479.f},
                render_config,
                scene_graph_config,
                render_results,
                {.external_render_pass = {ExternalRenderPassType::DIRTMAP, ""},
                 .time_id = 0,
                 .light_resource_suffix = ""});
        }
        // Load texture and set alias
        rendering_context_.rendering_resources->add_texture_descriptor(
            "dirtmap",
            TextureDescriptor{
                .color = {.filename = filename_},
                .color_mode = ColorMode::GRAYSCALE,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                .anisotropic_filtering_level = 8});
        rendering_context_.rendering_resources->set_vp("dirtmap", vp());
        generated_ = true;
    }
}

float DirtmapLogic::near_plane() const {
    return child_logic_.near_plane();
}

float DirtmapLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<double, 4, 4>& DirtmapLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, double, 3>& DirtmapLogic::iv() const {
    return child_logic_.iv();
}

bool DirtmapLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void DirtmapLogic::set_filename(const std::string& filename) {
    if (!filename_.empty()) {
        THROW_OR_ABORT("DirtmapLogic::set_filename called multiple times");
    }
    filename_ = filename;
}

void DirtmapLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "DirtmapLogic\n";
    child_logic_.print(ostr, depth + 1);
}
