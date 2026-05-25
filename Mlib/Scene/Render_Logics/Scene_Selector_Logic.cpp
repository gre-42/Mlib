#include "Scene_Selector_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/OpenGL/Render_Setup.hpp>
#include <Mlib/OpenGL/Text/Charsets.hpp>
#include <Mlib/OpenGL/Text/Renderable_Text.hpp>
#include <Mlib/OpenGL/Ui/Button_Press.hpp>
#include <Mlib/OpenGL/Ui/List_View_Orientation.hpp>
#include <Mlib/OpenGL/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Remote/Incremental_Objects/Scene_Level.hpp>
#include <Mlib/Scene/Scene_Reloader.hpp>

using namespace Mlib;

SceneEntryContents::SceneEntryContents(
    const std::vector<ReplacementParameterAndFilename>& scene_entries,
    const ExpressionWatcher& ew)
    : scene_entries_{scene_entries}
    , ew_{ew}
{}

size_t SceneEntryContents::num_entries() const {
    return scene_entries_.size();
}

bool SceneEntryContents::is_visible(size_t index) const {
    const auto& entry = scene_entries_.at(index);
    try {
        return ew_.eval(entry.rp.required.dynamic, entry.rp.database);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Could not evaluate dynamic visibility expression of entry \"" + entry.rp.id + "\": " + e.what());
    }
}

SceneSelectorLogic::SceneSelectorLogic(
    std::string id,
    std::vector<ReplacementParameterAndFilename> scene_files,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    std::unique_ptr<ExpressionWatcher>&& ew,
    SceneReloader& scene_reloader,
    ButtonStates& button_states,
    UiFocus& ui_focus,
    uint32_t local_user_id,
    std::function<void()> on_change)
    : ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , renderable_text_{ std::make_unique<TextResource>(
        ascii,
        std::move(ttf_filename),
        font_color) }
    , scene_files_{ std::move(scene_files) }
    , contents_{ scene_files_, *ew_ }
    , widget_{ std::move(widget) }
    , font_height_{ font_height }
    , line_distance_{ line_distance }
    , focus_filter_{ std::move(focus_filter) }
    , ui_focus_{ ui_focus }
    , id_{ std::move(id) }
    , list_view_{
        "id = " + id_,
        button_states,
        ui_focus.all_selection_ids.at(id_),
        contents_,
        ListViewOrientation::VERTICAL,
        ui_focus,
        local_user_id,
        [this, oc = std::move(on_change), sr = &scene_reloader]() {
            const auto& entry = scene_files_.at(list_view_.selected_element());
            sr->set_next_scene_by_manifest(entry);
            merge_substitutions();
            oc();
        } }
    , ot_{ ew_->add_observer([this]() {
        list_view_.notify_change_visibility();
        }) }
{
    scene_titles_.resize(scene_files_.size());
}

SceneSelectorLogic::~SceneSelectorLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> SceneSelectorLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void SceneSelectorLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("SceneSelectorLogic::render");
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    if (ew_->result_may_have_changed()) {
        for (const auto& [i, s] : enumerate(scene_files_)) {
            scene_titles_.at(i) = ew_->eval<std::string>(s.rp.title, s.rp.database);
        }
        renderable_text_->set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    ListViewStringDrawer drawer{
        ListViewOrientation::VERTICAL,
        *renderable_text_,
        font_height_,
        line_distance_,
        *ew,
        ly,
        [this](size_t index) {return scene_titles_.at(index);}};
    list_view_.render_and_handle_input(lx, ly, drawer);
    drawer.render();
}

bool SceneSelectorLogic::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void SceneSelectorLogic::merge_substitutions() const {
    ui_focus_.all_selection_ids[id_] = list_view_.selected_element();

    const auto& element = scene_files_.at(list_view_.selected_element());
    const auto& on_before_select = element.rp.on_before_select;
    if (!on_before_select.is_null()) {
        ew_->execute(on_before_select, nullptr);
    }
}

void SceneSelectorLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SceneSelectorLogic";
}
