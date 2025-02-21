#include "Parameter_Setter_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

ReplacementParameterContents::ReplacementParameterContents(
    const std::vector<ReplacementParameter>& options,
    const MacroLineExecutor& mle)
    : options_{ options }
    , mle_{ mle }
{}

size_t ReplacementParameterContents::num_entries() const {
    return options_.size();
}

bool ReplacementParameterContents::is_visible(size_t index) const {
    for (const auto& r : options_[index].required.dynamic) {
        if (!mle_.eval<bool>(r)) {
            return false;
        }
    }
    return true;
}

size_t selected_id(const std::string& id, const std::vector<ReplacementParameter>& options) {
    for (const auto& [i, o] : enumerate(options)) {
        if (o.id == id) {
            return i;
        }
    }
    return 0;
}

ParameterSetterLogic::ParameterSetterLogic(
    std::string id,
    std::vector<ReplacementParameter> options,
    ButtonPress& confirm_button,
    VariableAndHash<std::string> charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    MacroLineExecutor mle,
    UiFocus& ui_focus,
    std::string persisted,
    ButtonStates& button_states,
    std::function<void()> on_change,
    std::function<void()> on_execute)
    : mle_{ std::move(mle) }
    , options_{ std::move(options) }
    , contents_{options_, mle_}
    , renderable_text_{std::make_unique<TextResource>(
        std::move(charset),
        std::move(ttf_filename),
        font_color)}
    , widget_{std::move(widget)}
    , font_height_{font_height}
    , line_distance_{line_distance}
    , focus_filter_{ std::move(focus_filter) }
    , confirm_button_{ confirm_button }
    , ui_focus_{ ui_focus }
    , persisted_{ std::move(persisted) }
    , id_{ std::move(id) }
    , on_execute_{ std::move(on_execute) }
    , list_view_{
        "id = " + id_,
        button_states,
        persisted_.empty()
            ? (size_t)ui_focus.all_selection_ids.at(id_)
            : selected_id(ui_focus.get_persisted_selection_id(persisted_), options_),
        contents_,
        ListViewOrientation::VERTICAL,
        [this, on_change](){
            merge_substitutions();
            on_change();
        }}
{
    mle_.add_observer([this](){
        list_view_.notify_change_visibility();
    });
}

ParameterSetterLogic::~ParameterSetterLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> ParameterSetterLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void ParameterSetterLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    {
        const auto& f = options_.at(list_view_.selected_element()).on_execute;
        if (!f.is_null() || on_execute_) {
            if (confirm_button_.keys_pressed()) {
                if (!f.is_null()) {
                    mle_(f, nullptr, nullptr);
                }
                if (on_execute_) {
                    on_execute_();
                }
            }
        }
    }
    LOG_FUNCTION("ParameterSetterLogic::render");
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    ListViewStringDrawer drawer{
          ListViewOrientation::VERTICAL,
          *renderable_text_,
          font_height_,
          line_distance_,
          *ew,
          ly,
          [this](size_t index) {return options_.at(index).title;}};
    list_view_.render_and_handle_input(lx, ly, drawer);
    drawer.render();
}

FocusFilter ParameterSetterLogic::focus_filter() const {
    return focus_filter_;
}

void ParameterSetterLogic::merge_substitutions() const {
    auto e = list_view_.selected_element();
    const auto& element = options_.at(e);

    if (persisted_.empty()) {
        ui_focus_.all_selection_ids[id_] = e;
    } else {
        ui_focus_.set_persisted_selection_id(persisted_, element.id, PersistedValueType::CUSTOM);
    }

    const auto& f = element.on_before_select;
    if (!f.is_null()) {
        mle_(f, nullptr, nullptr);
    }
}

void ParameterSetterLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ParameterSetterLogic\n";
}
