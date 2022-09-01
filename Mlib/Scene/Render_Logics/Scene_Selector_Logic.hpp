#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <vector>

namespace Mlib {

class ButtonPress;
class ThreadSafeString;

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
        const FocusFilter& focus_filter,
        ThreadSafeString& next_scene_filename,
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
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::vector<SceneEntry> scene_files_;
    ListView<SceneEntry> list_view_;
    FocusFilter focus_filter_;
    ThreadSafeString& next_scene_filename_;
};

}
