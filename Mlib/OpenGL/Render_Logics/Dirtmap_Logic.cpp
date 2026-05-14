#include "Dirtmap_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Aggregate_Array_Renderer.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Render_Setup.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Render/Batch_Renderers/Array_Instances_Renderer.hpp>
#include <Mlib/Scene_Graph/Render/Batch_Renderers/Array_Instances_Renderers.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <stdexcept>

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
        throw std::runtime_error("DirtmapLogic received dirtmap rendering");
    }
    if (!rendering_resources_.contains_alias(dirtmap_)) {
        return;
    }
    if (!generated_) {
        // Calculate camera position
        auto dirtmap_rsd = RenderedSceneDescriptor{ .external_render_pass = {frame_id.external_render_pass.observer, ExternalRenderPassType::DIRTMAP, std::chrono::steady_clock::now()} };
        auto setup = child_logic_.render_setup(lx, ly, dirtmap_rsd);
        {
            auto& gpu_object_factory = RenderingContextStack::primary_gpu_object_factory();
            auto& gpu_vertex_array_renderer = RenderingContextStack::primary_gpu_vertex_array_renderer();
            AggregateRendererGuard arg{
                nullptr,
                nullptr,
                std::make_shared<AggregateArrayRenderer>(rendering_resources_),
                std::make_shared<AggregateArrayRenderer>(rendering_resources_)};
            InstancesRendererGuard irg{
                nullptr,
                nullptr,
                std::make_shared<ArrayInstancesRenderers>(gpu_object_factory, gpu_vertex_array_renderer),
                std::make_shared<ArrayInstancesRenderer>(gpu_object_factory, gpu_vertex_array_renderer)};
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
