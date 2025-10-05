#include "Video_Mode_Selector_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Render_Logics/Window_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

using namespace Mlib;

VideoModeContents::VideoModeContents(
    const std::vector<VideoMode>& video_modes)
    : video_modes_{ video_modes }
{}

size_t VideoModeContents::num_entries() const {
    return video_modes_.size();
}

bool VideoModeContents::is_visible(size_t index) const {
    return true;
}

VideoModeSelectorLogic::VideoModeSelectorLogic(
    std::string id,
    WindowLogic& window_logic,
    ButtonPress& confirm_button,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    std::unique_ptr<ExpressionWatcher>&& ew,
    ButtonStates& button_states,
    UiFocus& ui_focus,
    uint32_t user_id)
    : ew_{ std::move(ew) }
    , confirm_button_{ confirm_button }
    , charset_{ std::move(charset) }
    , renderable_text_{ std::make_unique<TextResource>(
        ascii,
        std::move(ttf_filename),
        font_color) }
    , contents_{ video_modes_ }
    , widget_{ std::move(widget) }
    , font_height_{ font_height }
    , line_distance_{ line_distance }
    , focus_filter_{ std::move(focus_filter) }
    , ui_focus_{ ui_focus }
    , id_{ std::move(id) }
    , window_logic_{ window_logic }
    , list_view_{
        "id = " + id_,
        button_states,
        ui_focus.all_selection_ids.at(id_),
        contents_,
        ListViewOrientation::VERTICAL,
        ui_focus,
        user_id}
{}

VideoModeSelectorLogic::~VideoModeSelectorLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> VideoModeSelectorLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void VideoModeSelectorLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("VideoModeSelectorLogic::render");
    if (confirm_button_.keys_pressed()) {
        if (list_view_.has_selected_element()) {
            const auto& mode = video_modes_.at(list_view_.selected_element());
            window_logic_.set_fullscreen_mode(DesiredVideoMode{
                mode.width,
                mode.height,
                mode.refresh_rate});
        }
    }
    video_modes_ = window_logic_.fullscreen_modes();
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    if (ew_->result_may_have_changed()) {
        renderable_text_->set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    ListViewStringDrawer drawer{
        ListViewOrientation::VERTICAL,
        *renderable_text_,
        font_height_,
        line_distance_,
        *ew,
        ly,
        [this](size_t index) {return video_modes_.at(index).to_string();}};
    list_view_.render_and_handle_input(lx, ly, drawer);
    drawer.render();
}

bool VideoModeSelectorLogic::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void VideoModeSelectorLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "VideoModeSelectorLogic";
}
