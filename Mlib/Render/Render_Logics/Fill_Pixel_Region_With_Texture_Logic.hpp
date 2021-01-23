#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;

class FillPixelRegionWithTextureLogic: public FillWithTextureLogic {
public:
    FillPixelRegionWithTextureLogic(
        const std::string& image_resource_name,
        ResourceUpdateCycle update_cycle,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        Focus focus_mask);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    
    virtual Focus focus_mask() const override;

private:
    FixedArray<float, 2> position_;
    FixedArray<float, 2> size_;
    Focus focus_mask_;
};

}
