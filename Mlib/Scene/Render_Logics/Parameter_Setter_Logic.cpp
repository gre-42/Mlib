#include "Parameter_Setter_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>

using namespace Mlib;

ReplacementParameterContents::ReplacementParameterContents(
    const std::vector<ReplacementParameter>& options,
    const SubstitutionMap& substitutions)
: options_{options},
  substitutions_{substitutions}
{}

size_t ReplacementParameterContents::num_entries() const {
    return options_.size();
}

bool ReplacementParameterContents::is_visible(size_t index) const {
    for (const auto& r : options_[index].requires_) {
        if (!substitutions_.get_bool(r)) {
            return false;
        }
    }
    return true;
}

ParameterSetterLogic::ParameterSetterLogic(
    size_t max_entry_distance,
    const std::string& title,
    std::vector<ReplacementParameter> options,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    SubstitutionMap& substitutions,
    ButtonPress& button_press,
    std::atomic_size_t& selection_index,
    const std::function<void()>& on_first_render,
    const std::function<void()>& on_change)
: options_{ std::move(options) },
  contents_{options_, substitutions},
  renderable_text_{std::make_unique<TextResource>(ttf_filename, font_height)},
  widget_{std::move(widget)},
  line_distance_{line_distance},
  list_view_{
    button_press,
    selection_index,
    max_entry_distance,
    contents_,
    ListViewOrientation::VERTICAL,
    [this, on_first_render](){
        if (!list_view_.has_selected_element()) {
            return;
        }
        merge_substitutions();
        on_first_render();
    },
    [this, on_change](){
        merge_substitutions();
        on_change();
    }},
  focus_filter_{ std::move(focus_filter) },
  substitutions_{ substitutions }
{
    if (list_view_.has_selected_element()) {
        merge_substitutions();
    }
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
          line_distance_,
          *ew,
          ly,
          [this](size_t index) {return options_.at(index).name;}};
    list_view_.render(lx, ly, drawer);
    drawer.render();
}

FocusFilter ParameterSetterLogic::focus_filter() const {
    return focus_filter_;
}

void ParameterSetterLogic::merge_substitutions() const {
    substitutions_.merge(options_.at(list_view_.selected_element()).variables);
}

void ParameterSetterLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ParameterSetterLogic\n";
}
