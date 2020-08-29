#include "Parameter_Setter_Logic.hpp"
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>

using namespace Mlib;

ParameterSetterLogic::ParameterSetterLogic(
    const std::vector<ReplacementParameter>& options,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    UiFocus& ui_focus,
    size_t submenu_id,
    std::string& substitutions,
    bool& leave_render_loop)
: scene_selector_list_view_{
    options,
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels,
    [](const ReplacementParameter& s){return s.name;}},
  ui_focus_{ui_focus},
  submenu_id_{submenu_id},
  substitutions_{substitutions},
  leave_render_loop_{leave_render_loop},
  window_{nullptr}
{}

ParameterSetterLogic::~ParameterSetterLogic()
{}

void ParameterSetterLogic::initialize(GLFWwindow* window) {
    scene_selector_list_view_.initialize(window);
    window_ = window;
}

void ParameterSetterLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    assert_true(window_ != nullptr);
    if (ui_focus_.focus == Focus::MENU) {
        if (ui_focus_.submenu_id == submenu_id_) {
            scene_selector_list_view_.handle_input();
            button_press_.update(window_);
            if (button_press_.key_pressed({key: "LEFT", joystick_axis: "1", joystick_axis_sign: -1})) {
                ui_focus_.goto_prev_submenu();
            }
            if (button_press_.key_pressed({key: "RIGHT", joystick_axis: "1", joystick_axis_sign: 1})) {
                ui_focus_.goto_next_submenu();
            }
            if (scene_selector_list_view_.has_selected_element()) {
                substitutions_ = scene_selector_list_view_.selected_element().substitutions;
            }
            if (button_press_.key_pressed({key: "ENTER", gamepad_button: "A"})) {
                ui_focus_.focus = Focus::LOADING;
                leave_render_loop_ = true;
            }
        }
    }
    if (ui_focus_.focus == Focus::MENU) {
        scene_selector_list_view_.render(width, height, true); // true=periodic_position
    }
}

float ParameterSetterLogic::near_plane() const {
    throw std::runtime_error("ParameterSetterLogic::requires_postprocessing not implemented");
}

float ParameterSetterLogic::far_plane() const {
    throw std::runtime_error("ParameterSetterLogic::requires_postprocessing not implemented");
}

const FixedArray<float, 4, 4>& ParameterSetterLogic::vp() const {
    throw std::runtime_error("ParameterSetterLogic::vp not implemented");
}

bool ParameterSetterLogic::requires_postprocessing() const {
    throw std::runtime_error("ParameterSetterLogic::requires_postprocessing not implemented");
}
