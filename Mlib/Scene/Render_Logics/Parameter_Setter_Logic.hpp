#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <atomic>
#include <memory>
#include <vector>

namespace Mlib {

class ButtonPress;
class IWidget;
class ILayoutPixels;
struct ReplacementParameter;

class ReplacementParameterContents: public IListViewContents {
public:
    explicit ReplacementParameterContents(
        const std::vector<ReplacementParameter>& options,
        const NotifyingSubstitutionMap& substitutions);

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;
private:
    const std::vector<ReplacementParameter>& options_;
    const NotifyingSubstitutionMap& substitutions_;
};

class ParameterSetterLogic: public RenderLogic {
public:
    ParameterSetterLogic(
        size_t max_entry_distance,
        const std::string& title,
        std::vector<ReplacementParameter> options,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        NotifyingSubstitutionMap& substitutions,
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        const std::function<void()>& on_first_render = [](){},
        const std::function<void()>& on_change = [](){});
    ~ParameterSetterLogic();

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
    void merge_substitutions() const;
    std::vector<ReplacementParameter> options_;
    ReplacementParameterContents contents_;
    std::unique_ptr<TextResource> renderable_text_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& line_distance_;
    ListView list_view_;
    FocusFilter focus_filter_;
    NotifyingSubstitutionMap& substitutions_;
};

}
