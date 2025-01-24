#include "Tab_Menu_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Render/Ui/List_View_Widget_Drawer.hpp>
#include <Mlib/Scene/Render_Logics/List_View_Style.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

using namespace Mlib;

SubmenuHeaderContents::SubmenuHeaderContents(
    const std::vector<SubmenuHeader>& options,
    const NotifyingJsonMacroArguments& substitutions,
    const AssetReferences& asset_references,
    UiFocus& ui_focus)
    : options_{ options }
    , substitutions_{ substitutions }
    , asset_references_{ asset_references }
    , ui_focus_{ ui_focus }
{}
    
size_t SubmenuHeaderContents::num_entries() const {
    return options_.size();
}

bool SubmenuHeaderContents::is_visible(size_t index) const {
    auto variables = substitutions_.json_macro_arguments();
    for (const auto& r : ui_focus_.submenu_headers.at(index).requires_) {
        if (!eval<bool>(r, variables, asset_references_)) {
            return false;
        }
    }
    return true;
}

TabMenuLogic::TabMenuLogic(
    std::string debug_hint,
    BaseKeyCombination key_binding,
    const std::vector<SubmenuHeader>& options,
    RenderLogicGallery& gallery,
    ListViewStyle list_view_style,
    const std::string& selection_marker,
    const std::string& ttf_filename,
    std::unique_ptr<IWidget>&& icon_widget,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    NotifyingJsonMacroArguments& substitutions,
    const AssetReferences& asset_references,
    UiFocus& ui_focus,
    std::atomic_size_t& num_renderings,
    ButtonStates& button_states,
    std::atomic_size_t& selection_index,
    std::function<void()> reload_transient_objects,
    const std::function<void()>& on_change)
    : confirm_button_press_{ button_states, key_configurations_, "confirm", "" }
    , renderable_text_{ std::make_unique<TextResource>(
        ttf_filename,
        font_color) }
    , options_{ options }
    , contents_{ options, substitutions, asset_references, ui_focus }
    , gallery_{ gallery }
    , list_view_style_{ list_view_style }
    , selection_marker_{ selection_marker }
    , icon_widget_{ std::move(icon_widget) }
    , widget_{ std::move(widget) }
    , font_height_{ font_height }
    , line_distance_{ line_distance }
    , substitutions_{ substitutions }
    , previous_level_id_{ substitutions.at<std::string>("loaded_level_id", "") }
    , num_renderings_{ num_renderings }
    , reload_transient_objects_{ std::move(reload_transient_objects) }
    , list_view_{
        std::move(debug_hint),
        button_states,
        ui_focus.submenu_number,
        contents_,
        ListViewOrientation::HORIZONTAL,
        on_change }
{
    key_configurations_.insert("confirm", { std::move(key_binding) });
}

TabMenuLogic::~TabMenuLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> TabMenuLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void TabMenuLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("TabMenuLogic::render");
    if (confirm_button_press_.keys_pressed()) {
        // ui_focus_.focus.pop_back();
        if (previous_level_id_ != substitutions_.at<std::string>("selected_level_id")) {
            num_renderings_ = 0;
        }
        else {
            reload_transient_objects_();
        }
    }
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    if (list_view_style_ == ListViewStyle::TEXT) {
        ListViewStringDrawer drawer{
            ListViewOrientation::HORIZONTAL,
            *renderable_text_,
            font_height_,
            line_distance_,
            *ew,
            ly,
            [this](size_t index) {return options_.at(index).title;}};
        list_view_.render_and_handle_input(lx, ly, drawer);
        drawer.render();
    } else if (list_view_style_ == ListViewStyle::ICON) {
        if (icon_widget_ == nullptr) {
            THROW_OR_ABORT("Listview style is \"icon\", but icon widget is null");
        }
        auto iw = icon_widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
        ListViewWidgetDrawer drawer{
            [&](const IPixelRegion& ew){
                gallery_["dots_icon"]->render(
                    LayoutConstraintParameters::child_x(lx, ew),
                    LayoutConstraintParameters::child_y(ly, ew));
            },
            [&](const IPixelRegion& ew){
                gallery_["dots_icon"]->render(
                    LayoutConstraintParameters::child_x(lx, ew),
                    LayoutConstraintParameters::child_y(ly, ew));
            },
            [&](const IPixelRegion& ew, size_t index, bool is_selected){
                gallery_[options_.at(index).icon]->render(
                    LayoutConstraintParameters::child_x(lx, ew),
                    LayoutConstraintParameters::child_y(ly, ew));
                if (is_selected) {
                    gallery_[selection_marker_]->render(
                        LayoutConstraintParameters::child_x(lx, ew),
                        LayoutConstraintParameters::child_y(ly, ew));
                }
            },
            ListViewOrientation::HORIZONTAL,
            ew->width(),
            0.f, // margin
            *iw};
        list_view_.render_and_handle_input(lx, ly, drawer);
    } else {
        THROW_OR_ABORT("Unknown listview style");
    }
}

FocusFilter TabMenuLogic::focus_filter() const {
    return { .focus_mask = Focus::MENU };
}

void TabMenuLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "TabMenuLogic\n";
}
