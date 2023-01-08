#include "Scene_Selector_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

SceneSelectorLogic::SceneSelectorLogic(
    const std::string& title,
    std::vector<SceneEntry> scene_files,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    float font_height,
    float line_distance,
    ScreenUnits units,
    FocusFilter focus_filter,
    SubstitutionMap& substitutions,
    ThreadSafeString& next_scene_filename,
    ButtonPress& button_press,
    std::atomic_size_t& selection_index,
    const std::function<void()>& on_change)
: scene_files_{ std::move(scene_files) },
  list_view_{
    button_press,
    selection_index,
    title,
    scene_files_,
    ttf_filename,
    position,
    size,
    font_height,
    line_distance,
    units,
    ListViewOrientation::VERTICAL,
    [](const SceneEntry& s){return s.name;},
    std::function<void()>(),
    [this, on_change](){
        next_scene_filename_ = list_view_.selected_element().filename;
        merge_substitutions();
        on_change();
    }},
  focus_filter_{ std::move(focus_filter) },
  substitutions_{ substitutions },
  next_scene_filename_{ next_scene_filename }
{
    if (list_view_.has_selected_element()) {
        if (((std::string)next_scene_filename_).empty()) {
            next_scene_filename_ = list_view_.selected_element().filename;
        }
        merge_substitutions();
    }
}

SceneSelectorLogic::~SceneSelectorLogic() = default;

void SceneSelectorLogic::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("SceneSelectorLogic::render");
    list_view_.handle_input();
    list_view_.render(width, height, xdpi, ydpi);
}

FocusFilter SceneSelectorLogic::focus_filter() const {
    return focus_filter_;
}

void SceneSelectorLogic::merge_substitutions() const {
    substitutions_.merge(MacroManifest{list_view_.selected_element().filename}.variables);
}

void SceneSelectorLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SceneSelectorLogic\n";
}
