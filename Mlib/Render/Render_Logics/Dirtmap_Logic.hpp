#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <memory>
#include <string>

namespace Mlib {

struct RenderingContext;
class RenderingResources;

class DirtmapLogic: public RenderLogic {
public:
    explicit DirtmapLogic(
        RenderingResources& rendering_resources,
        RenderLogic& child_logic);
    ~DirtmapLogic();

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
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderingResources& rendering_resources_;
    RenderLogic& child_logic_;
    bool generated_;
};

}
