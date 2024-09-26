#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;

class RenderToPercentageRegionLogic: public RenderLogic {
public:
    RenderToPercentageRegionLogic(
        RenderLogic& render_logic,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        FocusFilter focus_filter,
        bool flip_y = true);
    ~RenderToPercentageRegionLogic();

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
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    DestructionFunctionsRemovalTokens on_render_logic_destroy;
private:
    RenderLogic& render_logic_;
    FixedArray<float, 2> position_;
    FixedArray<float, 2> size_;
    FocusFilter focus_filter_;
    bool flip_y_;
};

}
