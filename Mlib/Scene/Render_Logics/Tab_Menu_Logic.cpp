#include "Tab_Menu_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

TabMenuLogic::TabMenuLogic(
    BaseKeyBinding key_binding,
    size_t max_entry_distance,
    const std::vector<SubmenuHeader>& options,
    const std::string& ttf_filename,
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
    const std::function<void()>& on_change)
: key_binding_{ std::move(key_binding) },
  renderable_text_{std::make_unique<TextResource>(ttf_filename, font_height)},
  options_{options},
  widget_{std::move(widget)},
  line_distance_{line_distance},
  substitutions_{substitutions},
  ui_focus_{ ui_focus },
  button_press_{ button_press },
  previous_scene_filename_{ std::move(previous_scene_filename) },
  next_scene_filename_{ next_scene_filename },
  num_renderings_{ num_renderings },
  reload_transient_objects_{ std::move(reload_transient_objects) },
  list_view_{
      button_press,
      ui_focus_.submenu_number,
      max_entry_distance,
      *this,
      ListViewOrientation::HORIZONTAL,
      std::function<void()>(),
      on_change}
{}

TabMenuLogic::~TabMenuLogic() = default;

size_t TabMenuLogic::num_entries() const {
    return options_.size();
}

bool TabMenuLogic::is_visible(size_t index) const {
    const auto& requires_ = ui_focus_.submenu_headers.at(index).requires_;
    return (requires_.empty() || substitutions_.get_bool(requires_));
}

void TabMenuLogic::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
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
    auto ew = widget_->evaluate(
        xdpi,
        ydpi,
        width,
        height,
        YOrientation::AS_IS);
    ListViewStringDrawer drawer{
        ListViewOrientation::HORIZONTAL,
        *renderable_text_,
        line_distance_,
        *ew,
        height,
        ydpi,
        [this](size_t index) {return options_.at(index).title;}};
    list_view_.render(width, height, xdpi, ydpi, drawer);
    drawer.render(
        width,
        height,
        xdpi,
        ydpi,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
}

FocusFilter TabMenuLogic::focus_filter() const {
    return { .focus_mask = Focus::MENU };
}

void TabMenuLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "TabMenuLogic\n";
}
