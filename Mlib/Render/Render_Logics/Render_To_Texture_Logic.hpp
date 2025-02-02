#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Variable_And_Hash.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;
enum class FrameBufferChannelKind;
class FrameBuffer;
class RenderingResources;

class RenderToTextureLogic: public RenderLogic {
public:
    explicit RenderToTextureLogic(
        RenderLogic& child_logic,
        RenderingResources& rendering_resources,
        ResourceUpdateCycle update_cycle,
        FrameBufferChannelKind depth_kind,
        VariableAndHash<std::string> color_texture_name,
        VariableAndHash<std::string> depth_texture_name,
        const FixedArray<int, 2>& texture_size,
        FocusFilter focus_filter);
    virtual ~RenderToTextureLogic();

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
    RenderLogic& child_logic_;
    RenderingResources& rendering_resources_;
    std::shared_ptr<FrameBuffer> fbs_;
    ResourceUpdateCycle update_cycle_;
    FrameBufferChannelKind depth_kind_;
    VariableAndHash<std::string> color_texture_name_;
    VariableAndHash<std::string> depth_texture_name_;
    FixedArray<int, 2> texture_size_;
    FocusFilter focus_filter_;
};

}
