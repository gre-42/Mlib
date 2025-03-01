#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

namespace Mlib {

class ButtonStates;
class IWidget;
class ILayoutPixels;
struct ReplacementParameter;
template <typename TData, size_t... tshape>
class FixedArray;
class KeyDescriptions;
class KeyConfigurations;

class KeyBindingsContents: public IListViewContents {
public:
    explicit KeyBindingsContents(
        std::string section,
        const KeyDescriptions& key_descriptions,
        const MacroLineExecutor& mle);

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;
private:
    std::string section_;
    const KeyDescriptions& key_descriptions_;
    const MacroLineExecutor& mle_;
};

class KeyBindingsLogic: public RenderLogic {
public:
    KeyBindingsLogic(
        std::string debug_hint,
        std::string section,
        const KeyDescriptions& key_descriptions,
        KeyConfigurations& key_configurations,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        MacroLineExecutor mle,
        ButtonStates& button_states,
        std::atomic_size_t& selection_index);
    ~KeyBindingsLogic();

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
    std::atomic_bool globals_changed_;
    std::string charset_;
    MacroLineExecutor mle_;
    const KeyDescriptions& key_descriptions_;
    KeyConfigurations& key_configurations_;
    KeyBindingsContents contents_;
    std::unique_ptr<TextResource> renderable_text_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& font_height_;
    const ILayoutPixels& line_distance_;
    FocusFilter focus_filter_;
    ListView list_view_;
};

}
