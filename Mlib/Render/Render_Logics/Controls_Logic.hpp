#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

class IWidget;
enum class ResourceUpdateCycle;

class ControlsLogic: public RenderLogic {
public:
    ControlsLogic(
        const std::string& gamepad_texture,
        std::unique_ptr<IWidget>&& widget,
        FocusFilter focus_filter);
    ~ControlsLogic();

    virtual void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    FillPixelRegionWithTextureLogic gamepad_texture_;
    FocusFilter focus_filter_;
};

}
