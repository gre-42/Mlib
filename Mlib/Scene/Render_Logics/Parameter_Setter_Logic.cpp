#include "Parameter_Setter_Logic.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

using namespace Mlib;

ParameterSetterLogic::ParameterSetterLogic(
    const std::vector<ReplacementParameter>& options,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    UiFocus& ui_focus,
    size_t submenu_id,
    SubstitutionString& substitutions,
    size_t& num_renderings,
    ButtonPress& button_press,
    size_t& selection_index,
    const std::function<void()>& on_change)
: scene_selector_list_view_{
    button_press,
    selection_index,
    options,
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels,
    [](const ReplacementParameter& s){return s.name;},
    [this, on_change](){
        merge_substitutions();
        on_change();
    }},
  ui_focus_{ui_focus},
  submenu_id_{submenu_id},
  substitutions_{substitutions},
  num_renderings_{num_renderings},
  button_press_{button_press}
{
    // Initialize the reference
    merge_substitutions();
}

ParameterSetterLogic::~ParameterSetterLogic()
{}

void ParameterSetterLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    if (ui_focus_.submenu_id == submenu_id_) {
        scene_selector_list_view_.handle_input();
        if (button_press_.key_pressed({key: "LEFT", joystick_axis: "1", joystick_axis_sign: -1})) {
            ui_focus_.goto_prev_submenu();
        }
        if (button_press_.key_pressed({key: "RIGHT", joystick_axis: "1", joystick_axis_sign: 1})) {
            ui_focus_.goto_next_submenu();
        }
        if (scene_selector_list_view_.has_selected_element()) {
            substitutions_.merge(scene_selector_list_view_.selected_element().substitutions);
        }
        if (button_press_.key_pressed({key: "ENTER", gamepad_button: "A"})) {
            // ui_focus_.focus.pop_back();
            num_renderings_ = 0;
        }
    }
    scene_selector_list_view_.render(width, height, true); // true=periodic_position
}

Focus ParameterSetterLogic::focus_mask() const {
    return Focus::MENU;
}

void ParameterSetterLogic::merge_substitutions() const {
    substitutions_.merge(scene_selector_list_view_.selected_element().substitutions);
}
