#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/ListView.hpp>
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
        const std::string& previous_scene_filename,
        std::string& next_scene_filename,
        size_t& num_renderings,
        ButtonPress& button_press,
        size_t& selection_index,
        const std::function<void()>& reload_transient_objects);
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
    ListView<SceneEntry> list_view_;
    UiFocus& ui_focus_;
    size_t submenu_id_;
    ButtonPress& button_press_;
    std::string previous_scene_filename_;
    std::string& next_scene_filename_;
    size_t& num_renderings_;
    std::function<void()> reload_transient_objects_;
};

}
