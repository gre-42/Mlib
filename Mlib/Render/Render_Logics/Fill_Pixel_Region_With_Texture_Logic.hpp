#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <memory>

namespace Mlib {

class FillWithTextureLogic;
class IWidget;
enum class DelayLoadPolicy;

class FillPixelRegionWithTextureLogic: public RenderLogic {
public:
    FillPixelRegionWithTextureLogic(
        std::shared_ptr<FillWithTextureLogic> fill_with_texture_logic,
        std::unique_ptr<IWidget>&& widget,
        DelayLoadPolicy delay_load_policy,
        FocusFilter focus_filter);
    ~FillPixelRegionWithTextureLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
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
    DelayLoadPolicy delay_load_policy_;
    FocusFilter focus_filter_;
};

}
