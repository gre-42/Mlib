#pragma once
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
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
    virtual bool is_visible(const UiFocus& ui_focus) const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    DestructionFunctionsRemovalTokens on_render_logic_destroy;
private:
    RenderLogic& render_logic_;
    std::unique_ptr<IWidget> widget_;
    FocusFilter focus_filter_;
};

}
