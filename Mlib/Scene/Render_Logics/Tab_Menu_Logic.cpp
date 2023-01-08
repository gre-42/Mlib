#include "Tab_Menu_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

TabMenuLogic::TabMenuLogic(
    BaseKeyBinding key_binding,
    const std::string& title,
    const std::vector<SubmenuHeader>& options,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    float font_height_pixels,
    float line_distance_pixels,
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
  ui_focus_{ ui_focus },
  button_press_{ button_press },
  previous_scene_filename_{ std::move(previous_scene_filename) },
  next_scene_filename_{ next_scene_filename },
  num_renderings_{ num_renderings },
  reload_transient_objects_{ std::move(reload_transient_objects) },
  list_view_{
      button_press,
      ui_focus_.submenu_number,
      title,
      options,
      ttf_filename,
      position,
      size,
      font_height_pixels,
      line_distance_pixels,
      ListViewOrientation::HORIZONTAL,
      [](const SubmenuHeader& header) {return header.title;},
      std::function<void()>(),
      on_change,
      [this, &substitutions](size_t submenu_number){
          const auto& requires_ = ui_focus_.submenu_headers.at(submenu_number).requires_;
          return (requires_.empty() || substitutions.get_bool(requires_));
      }}
{}

TabMenuLogic::~TabMenuLogic() = default;

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
    list_view_.render(width, height);
}

FocusFilter TabMenuLogic::focus_filter() const {
    return { .focus_mask = Focus::MENU };
}

void TabMenuLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "TabMenuLogic\n";
}
