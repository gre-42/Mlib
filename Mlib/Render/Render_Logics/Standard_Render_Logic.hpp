#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <memory>

namespace Mlib {

class Scene;
class RenderingResources;
class IAggregateRenderer;
class IInstancesRenderer;
class IInstancesRenderers;
enum class ClearMode;

class StandardRenderLogic: public RenderLogic {
public:
    StandardRenderLogic(
        RenderingResources& rendering_resources,
        const Scene& scene,
        RenderLogic& child_logic,
        const FixedArray<float, 3>& background_color,
        ClearMode clear_mode);
    ~StandardRenderLogic();

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
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void set_background_color(const FixedArray<float, 3>& color);
    void invalidate_aggregate_renderers();
private:
    const Scene& scene_;
    RenderLogic& child_logic_;
    FixedArray<float, 3> background_color_;
    ClearMode clear_mode_;
    std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer_;
    std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers_;
    std::shared_ptr<IAggregateRenderer> large_aggregate_renderer_;
    std::shared_ptr<IInstancesRenderer> large_instances_renderer_;
    SafeSharedMutex mutex_;
};

}
