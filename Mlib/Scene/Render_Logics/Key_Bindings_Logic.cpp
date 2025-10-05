#include "Key_Bindings_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Math/Sub_Sat.hpp>
#include <Mlib/Render/Key_Bindings/Input_Type.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Key_Description.hpp>
#include <Mlib/Render/Key_Bindings/Key_Descriptions.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Strings/Encoding.hpp>
#include <iomanip>
#include <sstream>

using namespace Mlib;

LockedKeyBindings::LockedKeyBindings(
    const LockableKeyDescriptions& key_descriptions,
    LockableKeyConfigurations& key_configurations)
    : descriptions{ key_descriptions.lock_shared() }
    , configurations{ key_configurations.lock_shared() }
{}

KeyBindingsContents::KeyBindingsContents(
    std::string section,
    const LockedKeyBindings& locked_key_bindings,
    const ExpressionWatcher& ew)
    : section_{ std::move(section) }
    , locked_key_bindings_{ locked_key_bindings }
    , ew_{ ew }
{}

size_t KeyBindingsContents::num_entries() const {
    return locked_key_bindings_.descriptions->size();
}

bool KeyBindingsContents::is_visible(size_t index) const {
    const auto& desc = *locked_key_bindings_.descriptions;
    const auto& k = desc[index];
    if (k.section != section_) {
        return false;
    }
    if (!ew_.eval(k.required)) {
        return false;
    }
    return true;
}

KeyBindingsLogic::KeyBindingsLogic(
    std::string debug_hint,
    std::string section,
    const LockableKeyDescriptions& key_descriptions,
    LockableKeyConfigurations& key_configurations,
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
    std::atomic_size_t& selection_index,
    uint32_t user_id)
    : user_id_{ user_id }
    , charset_{ std::move(charset) }
    , ew_{ std::move(ew) }
    , key_descriptions_{ key_descriptions }
    , key_configurations_{ key_configurations }
    , locked_key_bindings_{ key_descriptions_, key_configurations_ }
    , contents_{ std::move(section), locked_key_bindings_, *ew_ }
    , renderable_text_{std::make_unique<TextResource>(
        ascii,
        std::move(ttf_filename),
        font_color)}
    , widget_{std::move(widget)}
    , font_height_{font_height}
    , line_distance_{line_distance}
    , focus_filter_{ std::move(focus_filter) }
    , list_view_{
        std::move(debug_hint),
        button_states,
        selection_index,
        contents_,
        ListViewOrientation::VERTICAL,
        ui_focus,
        user_id}
    , ot_{ ew_->add_observer([this](){
        list_view_.notify_change_visibility();
    }) }
{}

KeyBindingsLogic::~KeyBindingsLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> KeyBindingsLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void KeyBindingsLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("KeyBindingsLogic::render");
    auto ew = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    auto s = ew_->eval<std::string>("%input_type_" + std::to_string(user_id_));
    auto filter = input_type_from_string(s);
    if (ew_->result_may_have_changed()) {
        renderable_text_->set_charset(VariableAndHash{ew_->eval<std::string>(charset_)});
    }
    const auto& desc = *locked_key_bindings_.descriptions;
    const auto& conf = *locked_key_bindings_.configurations;
    ListViewStringDrawer drawer{
        ListViewOrientation::VERTICAL,
        *renderable_text_,
        font_height_,
        line_distance_,
        *ew,
        ly,
        [this, filter, &desc, &conf](size_t index)
        {
            const auto& d = desc[index];
            const auto* k = conf.try_get(user_id_, d.id);
            if (k == nullptr) {
                return d.title;
            } else {
                return d.title + std::string(sub_sat<size_t>(40, nchars32(d.title)), ' ') + ": " + k->to_string(filter);
            }
            // std::basic_stringstream<char32_t> sstr;
            // sstr << std::left << std::setw(40) << u8_to_u32_string(d.title);
            // return u32_to_u8_string(sstr.str()) + ": " + k.to_string(filter);
        }};
    list_view_.render_and_handle_input(lx, ly, drawer);
    drawer.render();
}

bool KeyBindingsLogic::is_visible(const UiFocus& ui_focus) const {
    std::shared_lock lock{ ui_focus.focuses.mutex };
    return ui_focus.has_focus(focus_filter_);
}

void KeyBindingsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "KeyBindingsLogic";
}
