#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <memory>

namespace Mlib {

class Scene;
class RenderingResources;
class AggregateRenderer;
class InstancesRenderer;
enum class ClearMode;

class StandardRenderLogic: public RenderLogic {
public:
    StandardRenderLogic(
        const Scene& scene,
        RenderLogic& child_logic,
        const FixedArray<float, 3>& background_color,
        ClearMode clear_mode);
    ~StandardRenderLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<double, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, double, 3>& iv() const override;
    virtual const SceneNode& camera_node() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void set_background_color(const FixedArray<float, 3>& color);
private:
    const Scene& scene_;
    RenderLogic& child_logic_;
    FixedArray<float, 3> background_color_;
    ClearMode clear_mode_;
    RenderingContext rendering_context_;
    std::shared_ptr<AggregateRenderer> small_sorted_aggregate_renderer_;
    std::shared_ptr<InstancesRenderer> small_instances_renderer_;
    std::shared_ptr<InstancesRenderer> black_small_instances_renderer_;
};

}
