#include "Scene_Selector_Logic.hpp"
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

SceneSelectorLogic::SceneSelectorLogic(
    const std::string& title,
    const std::vector<SceneEntry>& scene_files,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    const FocusFilter& focus_filter,
    std::string& next_scene_filename,
    ButtonPress& button_press,
    size_t& selection_index)
: scene_files_{ scene_files },
  list_view_ {
    button_press,
    selection_index,
    title,
    scene_files_,
    ttf_filename,
    position,
    font_height_pixels,
    line_distance_pixels,
    ListViewOrientation::VERTICAL,
    [](const SceneEntry& s){return s.name;}},
  focus_filter_{ focus_filter },
  next_scene_filename_{ next_scene_filename }
{
    // Initialize the reference
    next_scene_filename_ = list_view_.selected_element().filename;
}

SceneSelectorLogic::~SceneSelectorLogic()
{}

void SceneSelectorLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    list_view_.handle_input();
    if (list_view_.has_selected_element()) {
        next_scene_filename_ = list_view_.selected_element().filename;
    }
    list_view_.render(width, height, true); // true=periodic_position
}

FocusFilter SceneSelectorLogic::focus_filter() const {
    return focus_filter_;
}
