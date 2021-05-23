#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <vector>

namespace Mlib {

class ButtonPress;

struct SceneEntry {
    std::string name;
    std::string filename;
};

class SceneSelectorLogic: public RenderLogic {
public:
    SceneSelectorLogic(
        const std::string& title,
        const std::vector<SceneEntry>& scene_files,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        UiFocus& ui_focus,
        size_t submenu_id_,
        std::string& next_scene_filename,
        ButtonPress& button_press,
        size_t& selection_index);
    ~SceneSelectorLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

    virtual Focus focus_mask() const override;

private:
    std::vector<SceneEntry> scene_files_;
    ListView<SceneEntry> list_view_;
    UiFocus& ui_focus_;
    size_t submenu_id_;
    std::string& next_scene_filename_;
};

}
