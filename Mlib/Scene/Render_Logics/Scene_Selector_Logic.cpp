#include "Scene_Selector_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

SceneEntry::SceneEntry(const ReplacementParameterAndFilename& rpe)
    : rpe_{ rpe }
    , locals_(std::map<std::string, std::string>{{"id", id()}})
{}

const std::string& SceneEntry::id() const {
    return rpe_.rp.id;
}

const std::string& SceneEntry::name() const {
    return rpe_.rp.title;
}

const std::string& SceneEntry::filename() const {
    return rpe_.filename;
}

const JsonMacroArguments& SceneEntry::globals() const {
    return rpe_.rp.globals;
}

JsonView SceneEntry::locals() const {
    return JsonView{ locals_ };
}

const std::vector<std::string>& SceneEntry::required() const {
    return rpe_.rp.required;
}

bool SceneEntry::operator < (const SceneEntry& other) const {
    return name() < other.name();
}

SceneEntryContents::SceneEntryContents(
    const std::vector<SceneEntry>& scene_entries,
    const NotifyingJsonMacroArguments& substitutions,
    const AssetReferences& asset_references)
    : scene_entries_{scene_entries}
    , substitutions_{substitutions}
    , asset_references_{asset_references}
{}

size_t SceneEntryContents::num_entries() const {
    return scene_entries_.size();
}

bool SceneEntryContents::is_visible(size_t index) const {
    const auto& entry = scene_entries_[index];
    auto variables = substitutions_.json_macro_arguments();
    for (const auto& r : entry.required()) {
        if (!eval<bool>(r, variables, entry.locals(), asset_references_)) {
            return false;
        }
    }
    return true;
}

SceneSelectorLogic::SceneSelectorLogic(
    std::string debug_hint,
    std::string globals_prefix,
    std::vector<SceneEntry> scene_files,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    NotifyingJsonMacroArguments& substitutions,
    const AssetReferences& asset_references,
    ThreadSafeString& next_scene_filename,
    ButtonStates& button_states,
    std::atomic_size_t& selection_index,
    const std::function<void()>& on_change)
    : globals_prefix_{ std::move(globals_prefix) }
    , renderable_text_{ std::make_unique<TextResource>(
        ttf_filename,
        FixedArray<float, 3>{1.f, 1.f, 1.f}) }
    , scene_files_{ std::move(scene_files) }
    , contents_{ scene_files_, substitutions, asset_references }
    , widget_{ std::move(widget) }
    , font_height_{ font_height }
    , line_distance_{ line_distance }
    , focus_filter_{ std::move(focus_filter) }
    , substitutions_{ substitutions }
    , next_scene_filename_{ next_scene_filename }
    , list_view_{
        std::move(debug_hint),
        button_states,
        selection_index,
        contents_,
        ListViewOrientation::VERTICAL,
        [this, on_change]() {
            next_scene_filename_ = scene_files_.at(list_view_.selected_element()).filename();
            merge_substitutions();
            on_change();
        } }
{
    substitutions_.add_observer([this]() {
        list_view_.notify_change_visibility();
        });
}

SceneSelectorLogic::~SceneSelectorLogic() {
    on_destroy.clear();
}

void SceneSelectorLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("SceneSelectorLogic::render");
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS);
    ListViewStringDrawer drawer{
        ListViewOrientation::VERTICAL,
        *renderable_text_,
        font_height_,
        line_distance_,
        *ew,
        ly,
        [this](size_t index) {return scene_files_.at(index).name();}};
    list_view_.render_and_handle_input(lx, ly, drawer);
    drawer.render();
}

FocusFilter SceneSelectorLogic::focus_filter() const {
    return focus_filter_;
}

void SceneSelectorLogic::merge_substitutions() const {
    const auto& element = scene_files_.at(list_view_.selected_element());
    auto globals = element.globals();
    globals.set(globals_prefix_ + "LEVEL_ID", element.id());
    substitutions_.merge_and_notify(globals);
}

void SceneSelectorLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SceneSelectorLogic\n";
}
