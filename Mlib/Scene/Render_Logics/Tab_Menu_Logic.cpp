#include "Tab_Menu_Logic.hpp"
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

TabMenuLogic::TabMenuLogic(
    const std::string& title,
    const std::vector<std::string>& options,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    UiFocus& ui_focus,
    size_t& num_renderings,
    ButtonPress& button_press,
    size_t& selection_index,
    const std::string& previous_scene_filename,
    const std::string& next_scene_filename,
    const std::function<void()>& reload_transient_objects,
    const std::function<void()>& on_change)
: ui_focus_{ui_focus},
  button_press_{button_press},
  previous_scene_filename_{previous_scene_filename},
  next_scene_filename_{next_scene_filename},
  num_renderings_{num_renderings},
  reload_transient_objects_{ reload_transient_objects },
  list_view_{
      button_press,
      ui_focus_.submenu_number,
      title,
      options,
      ttf_filename,
      position,
      font_height_pixels,
      line_distance_pixels,
      ListViewOrientation::HORIZONTAL,
      [](const std::string& name) {return name; },
      on_change }
{}

TabMenuLogic::~TabMenuLogic()
{}

void TabMenuLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    list_view_.handle_input();
    if (button_press_.key_pressed({ .key = "ENTER", .gamepad_button = "A" })) {
        // ui_focus_.focus.pop_back();
        if (previous_scene_filename_ != next_scene_filename_) {
            num_renderings_ = 0;
        }
        else {
            reload_transient_objects_();
        }
    }
    list_view_.render(width, height, true); // true=periodic_position
}

FocusFilter TabMenuLogic::focus_filter() const {
    return { .focus_mask = Focus::MENU };
}

void TabMenuLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "TabMenuLogic\n";
}
