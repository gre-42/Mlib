#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <atomic>
#include <functional>
#include <vector>

namespace Mlib {

struct UiFocus;
struct SubmenuHeader;
class ButtonPress;
class ThreadSafeString;
class SubstitutionMap;
class IWidget;
class ILayoutPixels;
class RenderLogicGallery;
enum class ListViewStyle;

struct TabEntry {
    std::string title;
    std::unique_ptr<RenderLogic> content;
};

class TabMenuLogic: public RenderLogic, public IListViewContents {
public:
    TabMenuLogic(
        BaseKeyBinding key_binding,
        size_t max_entry_distance,
        const std::vector<SubmenuHeader>& options,
        RenderLogicGallery& gallery,
        ListViewStyle list_view_style,
        const std::string& selection_marker,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& icon_widget,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        SubstitutionMap& substitutions,
        UiFocus& ui_focus,
        std::atomic_size_t& num_renderings,
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        std::string previous_scene_filename,
        const ThreadSafeString& next_scene_filename,
        std::function<void()> reload_transient_objects,
        const std::function<void()>& on_change = [](){});
    ~TabMenuLogic();

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;

    // RenderLogic
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
    BaseKeyBinding key_binding_;
    std::unique_ptr<TextResource> renderable_text_;
    const std::vector<SubmenuHeader>& options_;
    RenderLogicGallery& gallery_;
    ListViewStyle list_view_style_;
    std::string selection_marker_;
    std::unique_ptr<IWidget> icon_widget_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& line_distance_;
    const SubstitutionMap& substitutions_;
    UiFocus& ui_focus_;
    ButtonPress& button_press_;
    std::string previous_scene_filename_;
    const ThreadSafeString& next_scene_filename_;
    std::atomic_size_t& num_renderings_;
    std::function<void()> reload_transient_objects_;
    ListView list_view_;
};

}
