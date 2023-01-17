#include "Parameter_Setter_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>

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
  renderable_text_{std::make_unique<TextResource>(ttf_filename, font_height)},
  widget_{std::move(widget)},
  line_distance_{line_distance},
  list_view_{
    button_press,
    selection_index,
    max_entry_distance,
    *this,
    ListViewOrientation::VERTICAL,
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

size_t ParameterSetterLogic::num_entries() const {
    return options_.size();
}

bool ParameterSetterLogic::is_visible(size_t index) const {
    return true;
}

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
        substitutions_.merge(options_.at(list_view_.selected_element()).substitutions);
    }
    auto ew = widget_->evaluate(
        xdpi,
        ydpi,
        width,
        height,
        YOrientation::AS_IS);
    ListViewStringDrawer drawer{
          ListViewOrientation::VERTICAL,
          *renderable_text_,
          line_distance_,
          *ew,
          height,
          ydpi,
          [this](size_t index) {return options_.at(index).name;}};
    list_view_.render(width, height, xdpi, ydpi, drawer);
    drawer.render(height, ydpi);
}

FocusFilter ParameterSetterLogic::focus_filter() const {
    return focus_filter_;
}

void ParameterSetterLogic::merge_substitutions() const {
    substitutions_.merge(options_.at(list_view_.selected_element()).substitutions);
}

void ParameterSetterLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "ParameterSetterLogic\n";
}
