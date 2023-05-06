#include "Parameter_Setter_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>

using namespace Mlib;

ReplacementParameterContents::ReplacementParameterContents(
    const std::vector<ReplacementParameter>& options,
    const NotifyingJsonMacroArguments& substitutions)
: options_{options},
  substitutions_{substitutions}
{}

size_t ReplacementParameterContents::num_entries() const {
    return options_.size();
}

bool ReplacementParameterContents::is_visible(size_t index) const {
    for (const auto& r : options_[index].required) {
        if (!substitutions_.at<bool>(r)) {
            return false;
        }
    }
    return true;
}

ParameterSetterLogic::ParameterSetterLogic(
    const std::string& title,
    std::vector<ReplacementParameter> options,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    NotifyingJsonMacroArguments& substitutions,
    ButtonPress& button_press,
    std::atomic_size_t& selection_index,
    const std::function<void()>& on_change)
: options_{ std::move(options) },
  contents_{options_, substitutions},
  renderable_text_{std::make_unique<TextResource>(
    ttf_filename,
    FixedArray<float, 3>{1.f, 1.f, 1.f})},
  widget_{std::move(widget)},
  font_height_{font_height},
  line_distance_{line_distance},
  focus_filter_{ std::move(focus_filter) },
  substitutions_{ substitutions },
  list_view_{
    button_press,
    selection_index,
    contents_,
    ListViewOrientation::VERTICAL,
    [this, on_change](){
        merge_substitutions();
        on_change();
    }}
{
    substitutions_.add_observer([this](){
        list_view_.notify_change_visibility();
    });
}

ParameterSetterLogic::~ParameterSetterLogic() = default;

void ParameterSetterLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ParameterSetterLogic::render");
    list_view_.handle_input();
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS);
    ListViewStringDrawer drawer{
          ListViewOrientation::VERTICAL,
          *renderable_text_,
          font_height_,
          line_distance_,
          *ew,
          ly,
          [this](size_t index) {return options_.at(index).title;}};
    list_view_.render(lx, ly, drawer);
    drawer.render();
}

FocusFilter ParameterSetterLogic::focus_filter() const {
    return focus_filter_;
}

void ParameterSetterLogic::merge_substitutions() const {
    substitutions_.merge_and_notify(options_.at(list_view_.selected_element()).globals);
}

void ParameterSetterLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ParameterSetterLogic\n";
}
