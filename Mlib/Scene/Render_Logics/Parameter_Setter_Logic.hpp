#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

namespace Mlib {

template <class T>
class VariableAndHash;
class UiFocus;
class ButtonStates;
class IWidget;
class ILayoutPixels;
struct ReplacementParameter;
template <typename TData, size_t... tshape>
class FixedArray;

class ReplacementParameterContents: public IListViewContents {
public:
    explicit ReplacementParameterContents(
        const std::vector<ReplacementParameter>& options,
        const MacroLineExecutor& mle);

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;
private:
    const std::vector<ReplacementParameter>& options_;
    const MacroLineExecutor& mle_;
};

class ParameterSetterLogic: public RenderLogic {
public:
    ParameterSetterLogic(
        std::string id,
        std::vector<ReplacementParameter> options,
        ButtonPress& confirm_button,
        VariableAndHash<std::string> charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        MacroLineExecutor mle,
        UiFocus& ui_focus,
        std::string persisted,
        ButtonStates& button_states,
        const std::function<void()> on_change = [](){},
        const std::function<void()> on_execute = [](){});
    ~ParameterSetterLogic();

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
    std::vector<ReplacementParameter> options_;
    ReplacementParameterContents contents_;
    std::unique_ptr<TextResource> renderable_text_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& font_height_;
    const ILayoutPixels& line_distance_;
    FocusFilter focus_filter_;
    ButtonPress& confirm_button_;
    UiFocus& ui_focus_;
    std::string persisted_;
    std::string id_;
    std::function<void()> on_execute_;
    ListView list_view_;
};

}
