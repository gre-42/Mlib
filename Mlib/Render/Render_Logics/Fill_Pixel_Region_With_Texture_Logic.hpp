#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <memory>

namespace Mlib {

class IWidget;

class FillPixelRegionWithTextureLogic: public RenderLogic {
public:
    FillPixelRegionWithTextureLogic(
        std::shared_ptr<FillWithTextureLogic> fill_with_texture_logic,
        std::unique_ptr<IWidget>&& widget,
        FocusFilter focus_filter);

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
    std::shared_ptr<FillWithTextureLogic> fill_with_texture_logic_;
    std::unique_ptr<IWidget> widget_;
    FocusFilter focus_filter_;
};

}
