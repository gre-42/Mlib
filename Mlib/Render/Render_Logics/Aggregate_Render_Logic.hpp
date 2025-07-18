#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <memory>

namespace Mlib {

class RenderingResources;
class IAggregateRenderer;
class IInstancesRenderer;
class IInstancesRenderers;

class AggregateRenderLogic: public RenderLogic {
public:
    AggregateRenderLogic(
        RenderingResources& rendering_resources,
        RenderLogic& child_logic);
    ~AggregateRenderLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual bool render_optional_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup* setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void invalidate_aggregate_renderers();
    void wait_until_done();
    void stop_and_join();
private:
    RenderLogic& child_logic_;
    BackgroundLoop small_aggregate_bg_worker_;
    BackgroundLoop large_aggregate_bg_worker_;
    BackgroundLoop small_instances_bg_worker_;
    BackgroundLoop large_instances_bg_worker_;
    std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer_;
    std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers_;
    std::shared_ptr<IAggregateRenderer> large_aggregate_renderer_;
    std::shared_ptr<IInstancesRenderer> large_instances_renderer_;
    SafeAtomicRecursiveSharedMutex mutex_;
};

}
