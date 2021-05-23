#include "Parameter_Setter_Logic.hpp"
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

using namespace Mlib;

ParameterSetterLogic::ParameterSetterLogic(
    const std::string& title,
    const std::vector<ReplacementParameter>& options,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    UiFocus& ui_focus,
    size_t submenu_id,
    SubstitutionMap& substitutions,
    ButtonPress& button_press,
    size_t& selection_index,
    const std::function<void()>& on_change)
: options_{ options },
  list_view_ {
    button_press,
    selection_index,
    title,
    options_,
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels,
    ListViewOrientation::VERTICAL,
    [](const ReplacementParameter& s){return s.name;},
    [this, on_change](){
        merge_substitutions();
        on_change();
    }},
  ui_focus_{ui_focus},
  submenu_id_{submenu_id},
  substitutions_{substitutions}
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
        list_view_.handle_input();
        if (list_view_.has_selected_element()) {
            substitutions_.merge(list_view_.selected_element().substitutions);
        }
        list_view_.render(width, height, true); // true=periodic_position
    }
}

Focus ParameterSetterLogic::focus_mask() const {
    return Focus::MENU;
}

void ParameterSetterLogic::merge_substitutions() const {
    substitutions_.merge(list_view_.selected_element().substitutions);
}
