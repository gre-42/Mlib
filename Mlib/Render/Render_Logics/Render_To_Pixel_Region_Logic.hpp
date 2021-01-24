#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;

class RenderToPixelRegionLogic: public RenderLogic {
public:
    RenderToPixelRegionLogic(
        RenderLogic& render_logic,
        const FixedArray<int, 2>& position,
        const FixedArray<int, 2>& size,
        Focus focus_mask,
        bool flip_y = true);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    
    virtual Focus focus_mask() const override;

private:
    RenderLogic& render_logic_;
    FixedArray<int, 2> position_;
    FixedArray<int, 2> size_;
    Focus focus_mask_;
    bool flip_y_;
};

}
