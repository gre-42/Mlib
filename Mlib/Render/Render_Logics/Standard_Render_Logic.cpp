#include "Standard_Render_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Optional.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IAggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <mutex>

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
  small_sorted_aggregate_renderer_{IAggregateRenderer::small_sorted_aggregate_renderer()},
  small_sorted_instances_renderers_{IInstancesRenderer::small_sorted_instances_renderers()},
  large_aggregate_renderer_{IAggregateRenderer::large_aggregate_renderer()},
  large_instances_renderer_{IInstancesRenderer::large_instances_renderer()}
{}

StandardRenderLogic::~StandardRenderLogic() = default;

void StandardRenderLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("StandardRenderLogic::render");

    std::shared_lock lock{mutex_};
    RenderToScreenGuard rg;

    if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
        clear_color_and_depth({0.f, 0.f, 0.f, 1.f});
    } else if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        clear_color_and_depth({1.f, 1.f, 1.f, 1.f});
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::IMPOSTER_NODE) {
        clear_color_and_depth({
            background_color_(0),
            background_color_(1),
            background_color_(2),
            0.f});
    } else {
        if (clear_mode_ == ClearMode::COLOR) {
            clear_color({
                background_color_(0),
                background_color_(1),
                background_color_(2),
                1.f});
        } else if (clear_mode_ == ClearMode::DEPTH) {
            clear_depth();
        } else if (clear_mode_ == ClearMode::COLOR_AND_DEPTH) {
            clear_color_and_depth({
                background_color_(0),
                background_color_(1),
                background_color_(2),
                1.f});
        } else if (clear_mode_ == ClearMode::OFF) {
            // Do nothing
        } else {
            THROW_OR_ABORT("Unknown clear mode");
        }
    }

    {
        RenderingContextGuard rrg{ rendering_context_ };
        bool create_render_guards = !any(frame_id.external_render_pass.pass & ExternalRenderPassType::IS_STATIC_MASK);
        Optional<AggregateRendererGuard> arg(
            create_render_guards ? OptionalState::SOME : OptionalState::NONE,
            small_sorted_aggregate_renderer_,
            large_aggregate_renderer_);
        Optional<InstancesRendererGuard> irg(
            create_render_guards ? OptionalState::SOME : OptionalState::NONE,
            small_sorted_instances_renderers_,
            large_instances_renderer_);
        // Acquire delete node mutex because the "child_logic_.camera_node"
        // is read below.
        std::scoped_lock lock{ scene_.delete_node_mutex() };
        child_logic_.render(
            lx,
            ly,
            render_config,
            scene_graph_config,
            render_results, frame_id);

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
    //             THROW_OR_ABORT("Could not write to file " + sstr.str());
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
    std::scoped_lock lock{mutex_};
    background_color_ = color;
}

void StandardRenderLogic::invalidate_aggregate_renderers() {
    std::scoped_lock lock{mutex_};
    small_sorted_aggregate_renderer_->invalidate();
    small_sorted_instances_renderers_->invalidate();
    large_aggregate_renderer_->invalidate();
    large_instances_renderer_->invalidate();
}
