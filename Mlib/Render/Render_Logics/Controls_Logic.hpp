#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

class IWidget;
enum class ResourceUpdateCycle;
struct ColormapWithModifiers;

class ControlsLogic: public RenderLogic {
public:
    ControlsLogic(
        ColormapWithModifiers image_resource_name,
        std::unique_ptr<IWidget>&& widget,
        DelayLoadPolicy delay_load_policy,
        FocusFilter focus_filter);
    ~ControlsLogic();

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

private:
    FillPixelRegionWithTextureLogic gamepad_texture_;
    FocusFilter focus_filter_;
};

}
