#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;
enum class FrameBufferChannelKind;
class FrameBuffer;

class RenderToTextureLogic: public RenderLogic {
public:
    explicit RenderToTextureLogic(
        RenderLogic& child_logic,
        ResourceUpdateCycle update_cycle,
        FrameBufferChannelKind depth_kind,
        std::string color_texture_name,
        std::string depth_texture_name,
        const FixedArray<int, 2>& texture_size,
        FocusFilter focus_filter);
    ~RenderToTextureLogic();

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
    RenderLogic& child_logic_;
    RenderingContext rendering_context_;
    std::unique_ptr<FrameBuffer> fbs_;
    ResourceUpdateCycle update_cycle_;
    FrameBufferChannelKind depth_kind_;
    std::string color_texture_name_;
    std::string depth_texture_name_;
    FixedArray<int, 2> texture_size_;
    FocusFilter focus_filter_;
};

}
