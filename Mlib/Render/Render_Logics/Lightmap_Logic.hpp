#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <list>

namespace Mlib {

class Scene;
class RenderingResources;

enum class LightmapUpdateCycle {
    ONCE,
    ALWAYS
};

inline LightmapUpdateCycle lightmap_update_cycle_from_string(const std::string& str) {
    if (str == "once") {
        return LightmapUpdateCycle::ONCE;
    } else if (str == "always") {
        return LightmapUpdateCycle::ALWAYS;
    }
    throw std::runtime_error("Unknown lightmap update cycle");
}

class LightmapLogic: public RenderLogic {
public:
    explicit LightmapLogic(
        RenderLogic& child_logic,
        LightmapUpdateCycle update_cycle,
        const std::string& light_node_name,
        const std::string& black_node_name,
        bool with_depth_texture);
    ~LightmapLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, 3>& iv() const override;
    virtual bool requires_postprocessing() const override;

private:
    RenderLogic& child_logic_;
    std::shared_ptr<RenderingResources> rendering_resources_;
    std::unique_ptr<FrameBuffer> fb_;
    LightmapUpdateCycle update_cycle_;
    std::string light_node_name_;
    const std::string black_node_name_;
    bool with_depth_texture_;
};

}
