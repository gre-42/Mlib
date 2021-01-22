#include "Standard_Render_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Aggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Instances_Renderer.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

StandardRenderLogic::StandardRenderLogic(
    const Scene& scene,
    RenderLogic& child_logic,
    ClearMode clear_mode,
    Focus focus_mask)
: scene_{scene},
  child_logic_{child_logic},
  clear_mode_{clear_mode},
  focus_mask_{focus_mask},
  rendering_context_{RenderingContextStack::rendering_context()},
  small_sorted_aggregate_renderer_{AggregateRenderer::small_sorted_aggregate_renderer()},
  small_instances_renderer_{InstancesRenderer::small_instances_renderer()}
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

    if (frame_id.external_render_pass.pass == ExternalRenderPass::LIGHTMAP_TO_TEXTURE) {
        CHK(glClearColor(1.f, 1.f, 1.f, 1.f));
        CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    } else {
        GLbitfield mask = 0;
        if ((clear_mode_ == ClearMode::COLOR) || (clear_mode_ == ClearMode::COLOR_AND_DEPTH)) {
            CHK(glClearColor(
                render_config.background_color(0),
                render_config.background_color(1),
                render_config.background_color(2),
                1));
            mask |= GL_COLOR_BUFFER_BIT;
        }
        if ((clear_mode_ == ClearMode::DEPTH) || (clear_mode_ == ClearMode::COLOR_AND_DEPTH)) {
            mask |= GL_DEPTH_BUFFER_BIT;
        }
        CHK(glClear(mask));
    }

    {
        RenderingContextGuard rrg{rendering_context_};
        AggregateRendererGuard arg{small_sorted_aggregate_renderer_};
        InstancesRendererGuard irg{small_instances_renderer_};
        child_logic_.render(width, height, render_config, scene_graph_config, render_results, frame_id);

        render_config.apply();

        {
            auto primary_rendering_context = RenderingContextStack::primary_rendering_context();
            scene_.render(
                child_logic_.vp(),
                child_logic_.iv(),
                render_config,
                scene_graph_config,
                frame_id.external_render_pass,
                RenderingContextStack::generate_thread_runner(
                    primary_rendering_context,
                    rendering_context_));
        }

        render_config.unapply();
    }

    // if (frame_id.external_render_pass.pass == ExternalRenderPass::Pass::STANDARD_WO_POSTPROCESSING ||
    //     frame_id.external_render_pass.pass == ExternalRenderPass::Pass::STANDARD_WITH_POSTPROCESSING)
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

const FixedArray<float, 4, 4>& StandardRenderLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, 3>& StandardRenderLogic::iv() const {
    return child_logic_.iv();
}

bool StandardRenderLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

Focus StandardRenderLogic::focus_mask() const {
    return focus_mask_;
}
