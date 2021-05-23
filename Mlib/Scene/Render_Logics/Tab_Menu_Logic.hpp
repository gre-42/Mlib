#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <functional>
#include <vector>

namespace Mlib {

struct UiFocus;
class ButtonPress;

struct TabEntry {
    std::string title;
    std::unique_ptr<RenderLogic> content;
};

class TabMenuLogic: public RenderLogic {
public:
    TabMenuLogic(
        const std::string& title,
        const std::vector<std::string>& options,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        UiFocus& ui_focus,
        size_t& num_renderings,
        ButtonPress& button_press,
        size_t& selection_index,
        const std::string& previous_scene_filename,
        const std::string& next_scene_filename,
        const std::function<void()>& reload_transient_objects,
        const std::function<void()>& on_change = [](){});
    ~TabMenuLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

    virtual Focus focus_mask() const override;

private:
    UiFocus& ui_focus_;
    ButtonPress& button_press_;
    std::string previous_scene_filename_;
    const std::string& next_scene_filename_;
    size_t& num_renderings_;
    std::function<void()> reload_transient_objects_;
    ListView<std::string> list_view_;
};

}
