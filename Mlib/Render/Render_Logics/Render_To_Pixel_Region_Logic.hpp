#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <memory>

namespace Mlib {

enum class ResourceUpdateCycle;
class IWidget;

class RenderToPixelRegionLogic: public RenderLogic {
public:
    RenderToPixelRegionLogic(
        RenderLogic& render_logic,
        std::unique_ptr<IWidget>&& widget,
        FocusFilter focus_filter);
    ~RenderToPixelRegionLogic();

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderLogic& render_logic_;
    std::unique_ptr<IWidget> widget_;
    FocusFilter focus_filter_;
};

}
