#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;

class RenderToPixelRegionLogic: public RenderLogic {
public:
    RenderToPixelRegionLogic(
        RenderLogic& render_logic,
        const FixedArray<int, 2>& position,
        const FixedArray<int, 2>& size,
        const FocusFilter& focus_filter,
        bool flip_y = true);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderLogic& render_logic_;
    FixedArray<int, 2> position_;
    FixedArray<int, 2> size_;
    FocusFilter focus_filter_;
    bool flip_y_;
};

}
