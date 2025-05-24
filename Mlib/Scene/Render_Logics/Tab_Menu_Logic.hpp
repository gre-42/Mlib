#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Mlib {

enum class Focus;
class UiFocus;
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
class ExpressionWatcher;

class SubmenuHeaderContents: public IListViewContents {
public:
    explicit SubmenuHeaderContents(
        const NotifyingJsonMacroArguments& substitutions,
        const AssetReferences& asset_references,
        Focus focus_mask,
        UiFocus& ui_focus);

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;
private:
    const NotifyingJsonMacroArguments& substitutions_;
    const AssetReferences& asset_references_;
    Focus focus_mask_;
    UiFocus& ui_focus_;
};

class TabMenuLogic: public RenderLogic {
public:
    TabMenuLogic(
        std::string debug_hint,
        Focus focus_mask,
        ButtonPress& confirm_button,
        RenderLogicGallery& gallery,
        ListViewStyle list_view_style,
        const std::string& selection_marker,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& reference_widget,
        std::unique_ptr<IWidget>&& icon_widget,
        std::unique_ptr<IWidget>&& title_widget,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        NotifyingJsonMacroArguments& substitutions,
        const AssetReferences& asset_references,
        std::unique_ptr<ExpressionWatcher>&& ew,
        UiFocus& ui_focus,
        std::atomic_size_t& num_renderings,
        ButtonStates& button_states,
        uint32_t user_id,
        std::function<void()> on_execute,
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
    void merge_substitutions() const;
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    std::string ttf_filename_;
    FixedArray<float, 3> font_color_;
    std::string id_;
    Focus focus_mask_;
    ButtonPress& confirm_button_;
    std::unique_ptr<TextResource> renderable_text_;
    UiFocus& ui_focus_;
    SubmenuHeaderContents contents_;
    RenderLogicGallery& gallery_;
    ListViewStyle list_view_style_;
    std::string selection_marker_;
    std::unique_ptr<IWidget> reference_widget_;
    std::unique_ptr<IWidget> icon_widget_;
    std::unique_ptr<IWidget> title_widget_;
    std::unique_ptr<IWidget> widget_;
    std::unordered_map<size_t, std::string> titles_;
    std::unordered_map<size_t, std::unique_ptr<TextResource>> title_resources_;
    const ILayoutPixels& font_height_;
    const ILayoutPixels& line_distance_;
    const NotifyingJsonMacroArguments& substitutions_;
    std::atomic_size_t& num_renderings_;
    std::function<void()> on_execute_;
    ListView list_view_;
};

}
