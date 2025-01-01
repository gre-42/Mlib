#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <atomic>
#include <cstddef>
#include <functional>
#include <vector>

namespace Mlib {

struct UiFocus;
struct SubmenuHeader;
class ButtonPress;
class ButtonStates;
class ThreadSafeString;
class NotifyingJsonMacroArguments;
class AssetReferences;
class IWidget;
class ILayoutPixels;
class RenderLogicGallery;
enum class ListViewStyle;
template <typename TData, size_t... tshape>
class FixedArray;

class SubmenuHeaderContents: public IListViewContents {
public:
    explicit SubmenuHeaderContents(
        const std::vector<SubmenuHeader>& options,
        const NotifyingJsonMacroArguments& substitutions,
        const AssetReferences& asset_references,
        UiFocus& ui_focus);

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;
private:
    const std::vector<SubmenuHeader>& options_;
    const NotifyingJsonMacroArguments& substitutions_;
    const AssetReferences& asset_references_;
    UiFocus& ui_focus_;
};

class TabMenuLogic: public RenderLogic {
public:
    TabMenuLogic(
        std::string debug_hint,
        BaseKeyCombination key_binding,
        const std::vector<SubmenuHeader>& options,
        RenderLogicGallery& gallery,
        ListViewStyle list_view_style,
        const std::string& selection_marker,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& icon_widget,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        NotifyingJsonMacroArguments& substitutions,
        const AssetReferences& asset_references,
        UiFocus& ui_focus,
        std::atomic_size_t& num_renderings,
        ButtonStates& button_states,
        std::atomic_size_t& selection_index,
        std::function<void()> reload_transient_objects,
        const std::function<void()>& on_change = [](){});
    ~TabMenuLogic();

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
    ButtonPress confirm_button_press_;
    std::unique_ptr<TextResource> renderable_text_;
    const std::vector<SubmenuHeader>& options_;
    SubmenuHeaderContents contents_;
    RenderLogicGallery& gallery_;
    ListViewStyle list_view_style_;
    std::string selection_marker_;
    std::unique_ptr<IWidget> icon_widget_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& font_height_;
    const ILayoutPixels& line_distance_;
    const NotifyingJsonMacroArguments& substitutions_;
    KeyConfigurations key_configurations_;
    std::string previous_level_id_;
    std::atomic_size_t& num_renderings_;
    std::function<void()> reload_transient_objects_;
    ListView list_view_;
};

}
