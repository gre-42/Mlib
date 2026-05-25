#pragma once
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/OpenGL/Render_Logic.hpp>
#include <Mlib/OpenGL/Ui/IList_View_Contents.hpp>
#include <Mlib/OpenGL/Ui/List_View.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace Mlib {

template <class T>
class VariableAndHash;
class UiFocus;
class ButtonStates;
class SceneReloader;
class IWidget;
class ILayoutPixels;
struct ReplacementParameterAndFilename;
struct ReplacementParameterRequired;
template <typename TData, size_t... tshape>
class FixedArray;
class ExpressionWatcher;
class JsonView;
class SceneReloader;

class SceneEntryContents: public IListViewContents {
public:
    explicit SceneEntryContents(
        const std::vector<ReplacementParameterAndFilename>& scene_entries,
        const ExpressionWatcher& ew);

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;
private:
    const std::vector<ReplacementParameterAndFilename>& scene_entries_;
    const ExpressionWatcher& ew_;
};

class SceneSelectorLogic: public RenderLogic {
public:
    SceneSelectorLogic(
        std::string id,
        std::vector<ReplacementParameterAndFilename> scene_files,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        std::unique_ptr<ExpressionWatcher>&& ew,
        SceneReloader& scene_reloader,
        ButtonStates& button_states,
        UiFocus& ui_focus,
        uint32_t local_user_id,
        std::function<void()> on_change = [](){});
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
    virtual bool is_visible(const UiFocus& ui_focus) const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    void merge_substitutions() const;
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    std::string globals_prefix_;
    std::unique_ptr<TextResource> renderable_text_;
    std::vector<ReplacementParameterAndFilename> scene_files_;
    std::vector<std::string> scene_titles_;
    SceneEntryContents contents_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& font_height_;
    const ILayoutPixels& line_distance_;
    FocusFilter focus_filter_;
    UiFocus& ui_focus_;
    std::string id_;
    ListView list_view_;
    JsonMacroArgumentsObserverToken ot_;
};

}
