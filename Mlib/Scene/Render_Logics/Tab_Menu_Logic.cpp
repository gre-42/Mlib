#include "Tab_Menu_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_And_Position.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Render/Ui/List_View_Widget_Drawer.hpp>
#include <Mlib/Scene/Render_Logics/List_View_Style.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

using namespace Mlib;

SubmenuHeaderContents::SubmenuHeaderContents(
    const NotifyingJsonMacroArguments& substitutions,
    const AssetReferences& asset_references,
    Focus focus_mask,
    UiFocus& ui_focus)
    : substitutions_{ substitutions }
    , asset_references_{ asset_references }
    , focus_mask_{ focus_mask }
    , ui_focus_{ ui_focus }
{}
    
size_t SubmenuHeaderContents::num_entries() const {
    return ui_focus_.submenu_headers.size();
}

bool SubmenuHeaderContents::is_visible(size_t index) const {
    if (!any(focus_mask_ & ui_focus_.focus_masks.at(index))) {
        return false;
    }
    auto variables = substitutions_.json_macro_arguments();
    for (const auto& r : ui_focus_.submenu_headers.at(index).requires_) {
        if (!eval<bool>(r, variables, asset_references_)) {
            return false;
        }
    }
    return true;
}

TabMenuLogic::TabMenuLogic(
    std::string id,
    Focus focus_mask,
    ButtonPress& confirm_button,
    RenderLogicGallery& gallery,
    ListViewStyle list_view_style,
    const std::string& selection_marker,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& reference_widget,
    std::unique_ptr<IWidget>&& icon_widget,
    std::unique_ptr<IWidget>&& title_widget,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    NotifyingJsonMacroArguments& substitutions,
    const AssetReferences& asset_references,
    MacroLineExecutor mle,
    UiFocus& ui_focus,
    std::atomic_size_t& num_renderings,
    ButtonStates& button_states,
    std::function<void()> reload_transient_objects,
    const std::function<void()>& on_change)
    : mle_{ std::move(mle) }
    , charset_{ std::move(charset) }
    , ttf_filename_{ std::move(ttf_filename) }
    , globals_changed_{ true }
    , font_color_{ font_color }
    , id_{ id }
    , focus_mask_{ focus_mask }
    , confirm_button_{ confirm_button }
    , renderable_text_{ std::make_unique<TextResource>(
        ascii,
        ttf_filename_,
        font_color) }
    , ui_focus_{ ui_focus }
    , contents_{ substitutions, asset_references, focus_mask, ui_focus }
    , gallery_{ gallery }
    , list_view_style_{ list_view_style }
    , selection_marker_{ selection_marker }
    , reference_widget_{ std::move(reference_widget) }
    , icon_widget_{ std::move(icon_widget) }
    , title_widget_{ std::move(title_widget) }
    , widget_{ std::move(widget) }
    , font_height_{ font_height }
    , line_distance_{ line_distance }
    , substitutions_{ substitutions }
    , num_renderings_{ num_renderings }
    , on_execute_{ std::move(reload_transient_objects) }
    , list_view_{
        "id = " + id_,
        button_states,
        (size_t)ui_focus.menu_selection_ids[id_],
        contents_,
        ListViewOrientation::HORIZONTAL,
        [this, on_change](){
            merge_substitutions();
            on_change();
        }}
{
    if ((icon_widget_ == nullptr) != (title_widget_ == nullptr)) {
        THROW_OR_ABORT("Inconsistent icon / title widget definition");
    }
    mle_.add_observer([this](){
        globals_changed_ = true;
    });
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
    if (on_execute_ && confirm_button_.keys_pressed()) {
        on_execute_();
    }
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    if (list_view_style_ == ListViewStyle::TEXT) {
        if (globals_changed_) {
            renderable_text_->set_charset(VariableAndHash{mle_.eval<std::string>(charset_)});
        }
        ListViewStringDrawer drawer{
            ListViewOrientation::HORIZONTAL,
            *renderable_text_,
            font_height_,
            line_distance_,
            *ew,
            ly,
            [this](size_t index) {
                if (globals_changed_) {
                    titles_[index] = mle_.eval<std::string>(ui_focus_.submenu_headers.at(index).title);
                }
                return titles_.at(index);
            }};
        list_view_.render_and_handle_input(lx, ly, drawer);
        drawer.render();
    } else if (list_view_style_ == ListViewStyle::ICON) {
        if (reference_widget_ == nullptr) {
            THROW_OR_ABORT("Listview style is \"icon\", but reference widget is null");
        }
        auto r_ref = reference_widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
        auto r_icon = (icon_widget_ == nullptr) ? nullptr : icon_widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
        auto r_title = (title_widget_ == nullptr) ? nullptr : title_widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
        if (globals_changed_) {
            for (size_t i = 0; i < contents_.num_entries(); ++i) {
                titles_[i] = mle_.eval<std::string>(ui_focus_.submenu_headers.at(i).title);
                if (r_title != nullptr) {
                    auto it = title_resources_.find(i);
                    if (it == title_resources_.end()) {
                        title_resources_[i] = std::make_unique<TextResource>(ascii, ttf_filename_, font_color_);
                    }
                    title_resources_.at(i)->set_charset(VariableAndHash{mle_.eval<std::string>(charset_)});
                    title_resources_.at(i)->set_contents(
                        font_height_.to_pixels(ly, PixelsRoundMode::ROUND),
                        {r_title->width(), r_title->height()},
                        TextInterpolationMode::NEAREST_NEIGHBOR,
                        {TextAndPosition{
                        .text = titles_.at(i),
                        .position = {NAN, 0.f},
                        .align = VerticalTextAlignment::TOP,
                        .line_distance = line_distance_.to_pixels(ly, PixelsRoundMode::NONE)}});
                }
            }
        }
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
            [&](float dx, float dy, size_t index, bool is_selected){
                auto ew = PixelRegion::transformed(*r_ref, dx, dy);
                assert_true((r_icon == nullptr) == (r_title == nullptr));
                if (r_icon == nullptr) {
                    gallery_[ui_focus_.submenu_headers.at(index).icon]->render(
                        LayoutConstraintParameters::child_x(lx, ew),
                        LayoutConstraintParameters::child_y(ly, ew));
                } else {
                    if (is_selected) {
                        gallery_[selection_marker_]->render(
                            LayoutConstraintParameters::child_x(lx, ew),
                            LayoutConstraintParameters::child_y(ly, ew));
                    }
                    {
                        auto ei = PixelRegion::transformed(*r_icon, dx, dy);
                        gallery_[ui_focus_.submenu_headers.at(index).icon]->render(
                            LayoutConstraintParameters::child_x(lx, ei),
                            LayoutConstraintParameters::child_y(ly, ei));
                    }
                    {
                        auto et = PixelRegion::transformed(*r_title, dx, dy);
                        title_resources_.at(index)->render(et);
                    }
                }
            },
            ListViewOrientation::HORIZONTAL,
            ew->width(),
            0.f, // margin
            *r_ref};
        list_view_.render_and_handle_input(lx, ly, drawer);
    } else {
        THROW_OR_ABORT("Unknown listview style");
    }
    globals_changed_ = false;
}

FocusFilter TabMenuLogic::focus_filter() const {
    return { .focus_mask = focus_mask_ };
}

void TabMenuLogic::merge_substitutions() const {
    ui_focus_.menu_selection_ids[id_] = list_view_.selected_element();
}

void TabMenuLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "TabMenuLogic\n";
}
