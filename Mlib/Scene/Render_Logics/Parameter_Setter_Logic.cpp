#include "Parameter_Setter_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

using namespace Mlib;

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
  list_view_{
    button_press,
    selection_index,
    max_entry_distance,
    title,
    options_,
    ttf_filename,
    std::move(widget),
    font_height,
    line_distance,
    ListViewOrientation::VERTICAL,
    [](const ReplacementParameter& s){return s.name;},
    on_first_render,
    [this, on_change](){
        merge_substitutions();
        on_change();
    }},
  focus_filter_{ std::move(focus_filter) },
  substitutions_{ substitutions }
{
    merge_substitutions();
}

ParameterSetterLogic::~ParameterSetterLogic() = default;

void ParameterSetterLogic::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("ParameterSetterLogic::render");
    list_view_.handle_input();
    if (list_view_.has_selected_element()) {
        substitutions_.merge(list_view_.selected_element().substitutions);
    }
    list_view_.render(width, height, xdpi, ydpi);
}

FocusFilter ParameterSetterLogic::focus_filter() const {
    return focus_filter_;
}

void ParameterSetterLogic::merge_substitutions() const {
    substitutions_.merge(list_view_.selected_element().substitutions);
}

void ParameterSetterLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ParameterSetterLogic\n";
}
