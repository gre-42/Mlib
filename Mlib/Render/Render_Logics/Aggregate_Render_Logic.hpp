#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
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

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<double, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, double, 3>& iv() const override;
    virtual DanglingRef<const SceneNode> camera_node() const override;
    virtual bool requires_postprocessing() const override;
    virtual void reset() override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void invalidate_aggregate_renderers();
private:
    RenderLogic& child_logic_;
    std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer_;
    std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers_;
    std::shared_ptr<IAggregateRenderer> large_aggregate_renderer_;
    std::shared_ptr<IInstancesRenderer> large_instances_renderer_;
    SafeSharedMutex mutex_;
};

}
