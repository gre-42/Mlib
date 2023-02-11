#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
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
class IWidget;
class ILayoutPixels;

struct SceneEntry {
    std::string name;
    std::string filename;
    std::vector<std::string> requires_;
    std::strong_ordering operator <=> (const SceneEntry&) const = default;
};

class SceneSelectorLogic: public RenderLogic, public IListViewContents {
public:
    SceneSelectorLogic(
        const std::string& title,
        std::vector<SceneEntry> scene_files,
        size_t max_entry_distance,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        SubstitutionMap& substitutions,
        ThreadSafeString& next_scene_filename,
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        const std::function<void(SubstitutionMap& local_substitutions)>& on_init = [](SubstitutionMap& local_substitutions){},
        const std::function<void(SubstitutionMap& local_substitutions)>& on_change = [](SubstitutionMap& local_substitutions){});
    ~SceneSelectorLogic();

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;

    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    SubstitutionMap merge_substitutions() const;
    std::unique_ptr<TextResource> renderable_text_;
    std::vector<SceneEntry> scene_files_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& line_distance_;
    FocusFilter focus_filter_;
    SubstitutionMap& substitutions_;
    ThreadSafeString& next_scene_filename_;
    ListView list_view_;
};

}
