#pragma once
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <memory>

namespace Mlib {

class ObjectPool;
class FillWithTextureLogic;
class IWidget;
enum class DelayLoadPolicy;

class FillPixelRegionWithTextureLogic: public RenderLogic {
public:
    FillPixelRegionWithTextureLogic(
        ObjectPool* object_pool,
        DestructionFunctions* dependency_destruction_functions,
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
    virtual bool is_visible(const UiFocus& ui_focus) const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::shared_ptr<FillWithTextureLogic> fill_with_texture_logic_;
    std::unique_ptr<IWidget> widget_;
    DelayLoadPolicy delay_load_policy_;
    FocusFilter focus_filter_;
    DestructionFunctionsRemovalTokens on_delete_dependency_;
};

}
