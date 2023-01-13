#pragma once
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

class IWidget;
enum class ResourceUpdateCycle;

class FillPixelRegionWithTextureLogic: public FillWithTextureLogic {
public:
    FillPixelRegionWithTextureLogic(
        const std::string& image_resource_name,
        std::unique_ptr<IWidget>&& widget,
        ResourceUpdateCycle update_cycle,
        FocusFilter focus_filter);

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

private:
    std::unique_ptr<IWidget> widget_;
    FocusFilter focus_filter_;
};

}
