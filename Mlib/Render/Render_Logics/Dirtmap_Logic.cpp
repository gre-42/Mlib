#include "Dirtmap_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DirtmapLogic::DirtmapLogic(
    RenderingResources& rendering_resources,
    RenderLogic& child_logic)
    : rendering_resources_{ rendering_resources }
    , child_logic_{ child_logic }
    , generated_{ false }
    , dirtmap_{ "dirtmap" }
{}

DirtmapLogic::~DirtmapLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> DirtmapLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void DirtmapLogic::render_without_setup(
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
    if (!rendering_resources_.contains_alias(dirtmap_)) {
        return;
    }
    if (!generated_) {
        // Calculate camera position
        auto dirtmap_rsd = RenderedSceneDescriptor{ .external_render_pass = {ExternalRenderPassType::DIRTMAP, std::chrono::steady_clock::now()} };
        auto setup = child_logic_.render_setup(lx, ly, dirtmap_rsd);
        {
            AggregateRendererGuard arg{
                std::make_shared<AggregateArrayRenderer>(rendering_resources_),
                std::make_shared<AggregateArrayRenderer>(rendering_resources_)};
            InstancesRendererGuard irg{
                std::make_shared<ArrayInstancesRenderers>(rendering_resources_),
                std::make_shared<ArrayInstancesRenderer>(rendering_resources_)};
            child_logic_.render_with_setup(
                LayoutConstraintParameters{
                    .dpi = 96.f,
                    .min_pixel = 0.f,
                    .end_pixel = 640},
                LayoutConstraintParameters{
                    .dpi = 96.f,
                    .min_pixel = 0.f,
                    .end_pixel = 480},
                render_config,
                scene_graph_config,
                render_results,
                dirtmap_rsd,
                setup);
        }
        rendering_resources_.set_vp(dirtmap_, setup.vp);
        generated_ = true;
    }
}

void DirtmapLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "DirtmapLogic\n";
    child_logic_.print(ostr, depth + 1);
}
