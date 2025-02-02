#pragma once
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <atomic>
#include <vector>

namespace Mlib {

class ButtonStates;
class ThreadSafeString;
class IWidget;
class ILayoutPixels;
struct ReplacementParameterAndFilename;
struct ReplacementParameterRequired;
template <typename TData, size_t... tshape>
class FixedArray;

class SceneEntry {
public:
    explicit SceneEntry(const ReplacementParameterAndFilename& rpe);
    const std::string& id() const;
    const std::string& name() const;
    const std::string& filename() const;
    const nlohmann::json& on_before_select() const;
    const ReplacementParameterRequired& required() const;
    JsonView locals() const;
    bool operator < (const SceneEntry& other) const;
private:
    const ReplacementParameterAndFilename& rpe_;
    nlohmann::json locals_;
};

class SceneEntryContents: public IListViewContents {
public:
    explicit SceneEntryContents(
        const std::vector<SceneEntry>& scene_entries,
        const MacroLineExecutor& mle);

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;
private:
    const std::vector<SceneEntry>& scene_entries_;
    const MacroLineExecutor& mle_;
};

class SceneSelectorLogic: public RenderLogic {
public:
    SceneSelectorLogic(
        std::string debug_hint,
        std::vector<SceneEntry> scene_files,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        MacroLineExecutor mle,
        ThreadSafeString& next_scene_filename,
        ButtonStates& button_states,
        std::atomic_size_t& selection_index,
        const std::function<void()>& on_change = [](){});
    ~SceneSelectorLogic();

    // RenderLogic
    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    void merge_substitutions() const;
    MacroLineExecutor mle_;
    std::string globals_prefix_;
    std::unique_ptr<TextResource> renderable_text_;
    std::vector<SceneEntry> scene_files_;
    SceneEntryContents contents_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& font_height_;
    const ILayoutPixels& line_distance_;
    FocusFilter focus_filter_;
    ThreadSafeString& next_scene_filename_;
    ListView list_view_;
};

}
