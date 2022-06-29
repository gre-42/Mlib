#include "Standard_Render_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Aggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Instances_Renderer.hpp>

using namespace Mlib;

StandardRenderLogic::StandardRenderLogic(
    const Scene& scene,
    RenderLogic& child_logic,
    const FixedArray<float, 3>& background_color,
    ClearMode clear_mode)
: scene_{scene},
  child_logic_{child_logic},
  background_color_{background_color},
  clear_mode_{clear_mode},
  rendering_context_{RenderingContextStack::resource_context()},
  small_sorted_aggregate_renderer_{AggregateRenderer::small_sorted_aggregate_renderer()},
  small_sorted_instances_renderers_{InstancesRenderer::small_sorted_instances_renderers()},
  large_aggregate_renderer_{AggregateRenderer::large_aggregate_renderer()},
  large_instances_renderer_{InstancesRenderer::large_instances_renderer()}
{}

StandardRenderLogic::~StandardRenderLogic()
{}

void StandardRenderLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("StandardRenderLogic::render");
    RenderToScreenGuard rg;

    if (bool(frame_id.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
        CHK(glClearColor(0.f, 0.f, 0.f, 1.f));
        CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    } else if (bool(frame_id.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        CHK(glClearColor(1.f, 1.f, 1.f, 1.f));
        CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    } else {
        GLbitfield mask = 0;
        if ((clear_mode_ == ClearMode::COLOR) || (clear_mode_ == ClearMode::COLOR_AND_DEPTH)) {
            CHK(glClearColor(
                background_color_(0),
                background_color_(1),
                background_color_(2),
                1));
            mask |= GL_COLOR_BUFFER_BIT;
        }
        if ((clear_mode_ == ClearMode::DEPTH) || (clear_mode_ == ClearMode::COLOR_AND_DEPTH)) {
            mask |= GL_DEPTH_BUFFER_BIT;
        }
        CHK(glClear(mask));
    }

    {
        RenderingContextGuard rrg{ rendering_context_ };
        AggregateRendererGuard arg{
            small_sorted_aggregate_renderer_,
            large_aggregate_renderer_ };
        InstancesRendererGuard irg{
            small_sorted_instances_renderers_,
            large_instances_renderer_ };
        child_logic_.render(width, height, render_config, scene_graph_config, render_results, frame_id);

        RenderConfigGuard rcg{ render_config, frame_id.external_render_pass.pass };

        {
            auto primary_rendering_context = RenderingContextStack::primary_resource_context();
            scene_.render(
                child_logic_.vp(),
                child_logic_.iv(),
                child_logic_.camera_node(),
                render_config,
                scene_graph_config,
                frame_id.external_render_pass,
                RenderingContextStack::generate_thread_runner(
                    primary_rendering_context,
                    rendering_context_));
        }
    }

    // if (frame_id.external_render_pass.pass == ExternalRenderPassType::Pass::STANDARD_WO_POSTPROCESSING ||
    //     frame_id.external_render_pass.pass == ExternalRenderPassType::Pass::STANDARD_WITH_POSTPROCESSING)
    // {
    //     static Fps fps;
    //     fps.tick();
    //     static size_t ctr = 0;
    //     if (ctr++ % (60 * 5) == 0) {
    //         std::stringstream sstr;
    //         sstr << "/tmp/scene_"  <<
    //             std::setfill('0') <<
    //             std::setw(5) <<
    //             ctr <<
    //             "_" << 
    //             fps.fps() <<
    //             ".txt";
    //         std::ofstream f{sstr.str()};
    //         f << scene_ << std::endl;
    //         if (f.fail()) {
    //             throw std::runtime_error("Could not write to file " + sstr.str());
    //         }
    //     }
    // }
}

float StandardRenderLogic::near_plane() const {
    return child_logic_.near_plane();
}

float StandardRenderLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<double, 4, 4>& StandardRenderLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, double, 3>& StandardRenderLogic::iv() const {
    return child_logic_.iv();
}

const SceneNode& StandardRenderLogic::camera_node() const {
    return child_logic_.camera_node();
}

bool StandardRenderLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void StandardRenderLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "StandardRenderLogic\n";
    child_logic_.print(ostr, depth + 1);
}

void StandardRenderLogic::set_background_color(const FixedArray<float, 3>& color) {
    background_color_ = color;
}
