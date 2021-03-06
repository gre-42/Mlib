#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

enum class ResourceUpdateCycle;
struct FrameBufferMsaa;

class RenderToTextureLogic: public RenderLogic {
public:
    explicit RenderToTextureLogic(
        RenderLogic& child_logic,
        ResourceUpdateCycle update_cycle,
        bool with_depth_texture,
        const std::string& color_texture_name,
        const std::string& depth_texture_name,
        int texture_width,
        int texture_height,
        const FocusFilter& focus_filter);
    ~RenderToTextureLogic();

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
    RenderLogic& child_logic_;
    RenderingContext rendering_context_;
    std::unique_ptr<FrameBufferMsaa> fbs_;
    ResourceUpdateCycle update_cycle_;
    bool with_depth_texture_;
    std::string color_texture_name_;
    std::string depth_texture_name_;
    int texture_width_;
    int texture_height_;
    FocusFilter focus_filter_;
};

}
