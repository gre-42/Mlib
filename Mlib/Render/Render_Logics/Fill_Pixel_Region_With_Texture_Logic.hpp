#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

enum class ScreenUnits;
enum class ResourceUpdateCycle;

class FillPixelRegionWithTextureLogic: public FillWithTextureLogic {
public:
    FillPixelRegionWithTextureLogic(
        const std::string& image_resource_name,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        ScreenUnits screen_units,
        ResourceUpdateCycle update_cycle,
        FocusFilter focus_filter);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    
    virtual FocusFilter focus_filter() const override;

private:
    FixedArray<float, 2> position_;
    FixedArray<float, 2> size_;
    ScreenUnits screen_units_;
    FocusFilter focus_filter_;
};

}
