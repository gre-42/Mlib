#include "Scene_Selector_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

SceneEntryContents::SceneEntryContents(
    const std::vector<SceneEntry>& scene_entries,
    const NotifyingSubstitutionMap& substitutions)
: scene_entries_{scene_entries},
  substitutions_{substitutions}
{}

size_t SceneEntryContents::num_entries() const {
    return scene_entries_.size();
}

bool SceneEntryContents::is_visible(size_t index) const {
    for (const auto& r : scene_entries_[index].requires_) {
        if (!substitutions_.get_bool(r)) {
            return false;
        }
    }
    return true;
}

SceneSelectorLogic::SceneSelectorLogic(
    const std::string& title,
    std::vector<SceneEntry> scene_files,
    size_t max_entry_distance,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    NotifyingSubstitutionMap& substitutions,
    ThreadSafeString& next_scene_filename,
    ButtonPress& button_press,
    std::atomic_size_t& selection_index,
    const std::function<void()>& on_init,
    const std::function<void()>& on_change)
: renderable_text_{std::make_unique<TextResource>(ttf_filename, font_height)},
  scene_files_{ std::move(scene_files) },
  contents_{scene_files_, substitutions},
  widget_{std::move(widget)},
  line_distance_{line_distance},
  focus_filter_{ std::move(focus_filter) },
  substitutions_{ substitutions },
  next_scene_filename_{ next_scene_filename },
  list_view_{
    button_press,
    selection_index,
    max_entry_distance,
    contents_,
    ListViewOrientation::VERTICAL,
    std::function<void()>(),
    [this, on_change](){
        next_scene_filename_ = scene_files_.at(list_view_.selected_element()).filename;
        merge_substitutions();
        on_change();
    }}
{
    substitutions_.add_observer([this](){
        list_view_.notify_change_visibility();
    });
    if (list_view_.has_selected_element()) {
        if (((std::string)next_scene_filename_).empty()) {
            next_scene_filename_ = scene_files_.at(list_view_.selected_element()).filename;
        }
        merge_substitutions();
        on_init();
    }
}

SceneSelectorLogic::~SceneSelectorLogic() = default;

void SceneSelectorLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("SceneSelectorLogic::render");
    list_view_.handle_input();
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS);
    ListViewStringDrawer drawer{
        ListViewOrientation::VERTICAL,
        *renderable_text_,
        line_distance_,
        *ew,
        ly,
        [this](size_t index) {return scene_files_.at(index).name;}};
    list_view_.render(lx, ly, drawer);
    drawer.render();
}

FocusFilter SceneSelectorLogic::focus_filter() const {
    return focus_filter_;
}

void SceneSelectorLogic::merge_substitutions() const {
    substitutions_.merge_and_notify(scene_files_.at(list_view_.selected_element()).variables);
}

void SceneSelectorLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SceneSelectorLogic\n";
}
