#pragma once
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class RenderingResources;

class DirtmapLogic: public RenderLogic {
public:
    explicit DirtmapLogic(
        RenderLogic& child_logic,
        RenderingResources& rendering_resources);
    ~DirtmapLogic();

    virtual void initialize(GLFWwindow* window) override;
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
    virtual bool requires_postprocessing() const override;

    void set_filename(const std::string& filename);
private:
    RenderLogic& child_logic_;
    RenderingResources& rendering_resources_;
    bool generated_;
    std::string filename_;
};

}
