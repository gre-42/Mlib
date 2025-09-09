#include "Input_State_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <sstream>

using namespace Mlib;

InputStateLogic::InputStateLogic(
    std::unique_ptr<ExpressionWatcher>&& ew,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    std::chrono::milliseconds update_interval,
    const ButtonStates& button_states)
    : RenderTextLogic{
        ascii,
        std::move(ttf_filename),
        color,
        font_height,
        line_distance }
    , ew_{ std::move(ew) }
    , charset_{ std::move(charset) }
    , widget_{std::move(widget)}
    , last_update_{ std::chrono::steady_clock::time_point() }
    , update_interval_{ update_interval }
    , button_states_{ button_states }
    , focus_filter_{ std::move(focus_filter) }
{}

InputStateLogic::~InputStateLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> InputStateLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void InputStateLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("InputStateLogic::render");
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - last_update_;
    if ((last_update_ == std::chrono::steady_clock::time_point()) ||
        (elapsed > update_interval_))
    {
        std::stringstream sstr;
        button_states_.print(
            sstr,
            {
                .physical = false,
                .only_pressed = true,
                .min_deflection = BUTTON_STATES_MIN_DEFLECTION
            });
        text_ = sstr.str();
        last_update_ = now;
    }
    if (ew_->result_may_have_changed()) {
        renderable_text().set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    renderable_text().render(
        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
        *widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED),
        text_,
        line_distance_.to_pixels(ly, PixelsRoundMode::NONE),
        TextInterpolationMode::NEAREST_NEIGHBOR,
        GenericTextAlignment::DEFAULT,
        GenericTextAlignment::DEFAULT);
}

bool InputStateLogic::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void InputStateLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "InputStateLogic";
}
