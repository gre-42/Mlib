#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

namespace Mlib {

class RenderingResources;
enum class ResourceUpdateCycle;

class MainMenuBackgroundLogic: public FillWithTextureLogic {
public:
    MainMenuBackgroundLogic(
        const std::string& image_resource_name,
        ResourceUpdateCycle update_cycle,
        const FocusFilter& focus_filter);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    
    virtual FocusFilter focus_filter() const override;

private:
    FocusFilter focus_filter_;
};

}
