#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Advance_Time.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

namespace Mlib {

class RenderingResources;

class MainMenuBackgroundLogic: public FillWithTextureLogic {
public:
    MainMenuBackgroundLogic(
        RenderingResources& rendering_resources,
        const std::string& image_resource_name,
        const Focus& focus);

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

private:
    const Focus& focus_;
};

}
