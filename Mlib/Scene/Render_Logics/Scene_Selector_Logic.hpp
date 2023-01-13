#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <atomic>
#include <compare>
#include <vector>

namespace Mlib {

class ButtonPress;
class ThreadSafeString;
class SubstitutionMap;

struct SceneEntry {
    std::string name;
    std::string filename;
    std::strong_ordering operator <=> (const SceneEntry&) const = default;
};

class SceneSelectorLogic: public RenderLogic {
public:
    SceneSelectorLogic(
        const std::string& title,
        std::vector<SceneEntry> scene_files,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        float font_height,
        float line_distance,
        ScreenUnits units,
        FocusFilter focus_filter,
        SubstitutionMap& substitutions,
        ThreadSafeString& next_scene_filename,
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        const std::function<void()>& on_change = [](){});
    ~SceneSelectorLogic();

    virtual void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    void merge_substitutions() const;
    std::vector<SceneEntry> scene_files_;
    ListView<SceneEntry> list_view_;
    FocusFilter focus_filter_;
    SubstitutionMap& substitutions_;
    ThreadSafeString& next_scene_filename_;
};

}
