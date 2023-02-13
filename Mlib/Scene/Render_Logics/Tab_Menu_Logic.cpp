#include "Tab_Menu_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Render/Ui/List_View_Widget_Drawer.hpp>
#include <Mlib/Scene/Render_Logics/List_View_Style.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

SubmenuHeaderContents::SubmenuHeaderContents(
    const std::vector<SubmenuHeader>& options,
    const NotifyingSubstitutionMap& substitutions,
    UiFocus& ui_focus)
: options_{options},
  substitutions_{substitutions},
  ui_focus_{ui_focus}
{}
    
size_t SubmenuHeaderContents::num_entries() const {
    return options_.size();
}

bool SubmenuHeaderContents::is_visible(size_t index) const {
    const auto& requires_ = ui_focus_.submenu_headers.at(index).requires_;
    for (const auto& r : requires_) {
        if (!substitutions_.get_bool(r)) {
            return false;
        }
    }
    return true;
}

TabMenuLogic::TabMenuLogic(
    BaseKeyBinding key_binding,
    const std::vector<SubmenuHeader>& options,
    size_t max_entry_distance,
    RenderLogicGallery& gallery,
    ListViewStyle list_view_style,
    const std::string& selection_marker,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& icon_widget,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    NotifyingSubstitutionMap& substitutions,
    UiFocus& ui_focus,
    std::atomic_size_t& num_renderings,
    ButtonPress& button_press,
    std::atomic_size_t& selection_index,
    std::string previous_scene_filename,
    const ThreadSafeString& next_scene_filename,
    std::function<void()> reload_transient_objects,
    const std::function<void()>& on_change)
: key_binding_{ std::move(key_binding) },
  renderable_text_{std::make_unique<TextResource>(ttf_filename, font_height)},
  options_{options},
  contents_{options, substitutions, ui_focus},
  gallery_{gallery},
  list_view_style_{list_view_style},
  selection_marker_{selection_marker},
  icon_widget_{std::move(icon_widget)},
  widget_{std::move(widget)},
  line_distance_{line_distance},
  substitutions_{substitutions},
  button_press_{ button_press },
  previous_scene_filename_{ std::move(previous_scene_filename) },
  next_scene_filename_{ next_scene_filename },
  num_renderings_{ num_renderings },
  reload_transient_objects_{ std::move(reload_transient_objects) },
  list_view_{
      button_press,
      ui_focus.submenu_number,
      max_entry_distance,
      contents_,
      ListViewOrientation::HORIZONTAL,
      std::function<void()>(),
      on_change}
{}

TabMenuLogic::~TabMenuLogic() = default;

void TabMenuLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("TabMenuLogic::render");
    list_view_.handle_input();
    if (button_press_.key_pressed(key_binding_)) {
        // ui_focus_.focus.pop_back();
        if (previous_scene_filename_ != (std::string)next_scene_filename_) {
            num_renderings_ = 0;
        }
        else {
            reload_transient_objects_();
        }
    }
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS);
    if (list_view_style_ == ListViewStyle::TEXT) {
        ListViewStringDrawer drawer{
            ListViewOrientation::HORIZONTAL,
            *renderable_text_,
            line_distance_,
            *ew,
            ly,
            [this](size_t index) {return options_.at(index).title;}};
        list_view_.render(lx, ly, drawer);
        drawer.render();
    } else if (list_view_style_ == ListViewStyle::ICON) {
        if (icon_widget_ == nullptr) {
            THROW_OR_ABORT("Listview style is \"icon\", but icon widget is null");
        }
        auto iw = icon_widget_->evaluate(lx, ly, YOrientation::AS_IS);
        ListViewWidgetDrawer drawer{
            [&](const IPixelRegion& ew){
                gallery_["dots"]->render(
                    LayoutConstraintParameters::child_x(lx, ew),
                    LayoutConstraintParameters::child_y(ly, ew));
            },
            [&](const IPixelRegion& ew){
                gallery_["dots"]->render(
                    LayoutConstraintParameters::child_x(lx, ew),
                    LayoutConstraintParameters::child_y(ly, ew));
            },
            [&](const IPixelRegion& ew, size_t index, bool is_selected){
                gallery_[options_.at(index).icon]->render(
                    LayoutConstraintParameters::child_x(lx, ew),
                    LayoutConstraintParameters::child_y(ly, ew));
                if (is_selected) {
                    gallery_[selection_marker_]->render(
                        LayoutConstraintParameters::child_x(lx, ew),
                        LayoutConstraintParameters::child_y(ly, ew));
                }
            },
            ListViewOrientation::HORIZONTAL,
            ew->width(),
            0.f, // margin
            *iw};
        list_view_.render(lx, ly, drawer);
    } else {
        THROW_OR_ABORT("Unknown listview style");
    }
}

FocusFilter TabMenuLogic::focus_filter() const {
    return { .focus_mask = Focus::MENU };
}

void TabMenuLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "TabMenuLogic\n";
}
