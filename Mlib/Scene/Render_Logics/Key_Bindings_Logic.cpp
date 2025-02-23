#include "Key_Bindings_Logic.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Math/Sub_Sat.hpp>
#include <Mlib/Render/Key_Bindings/Input_Type.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Render/Key_Bindings/Key_Description.hpp>
#include <Mlib/Render/Key_Bindings/Key_Descriptions.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Text/Charsets.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Ui/List_View_String_Drawer.hpp>
#include <Mlib/Strings/Encoding.hpp>
#include <iomanip>
#include <sstream>

using namespace Mlib;

KeyBindingsContents::KeyBindingsContents(
    std::string section,
    const KeyDescriptions& key_descriptions,
    const MacroLineExecutor& mle)
    : section_{ std::move(section) }
    , key_descriptions_{ key_descriptions }
    , mle_{ mle }
{}

size_t KeyBindingsContents::num_entries() const {
    return key_descriptions_.size();
}

bool KeyBindingsContents::is_visible(size_t index) const {
    const auto& k = key_descriptions_[index];
    if (k.section != section_) {
        return false;
    }
    for (const auto& r : k.required) {
        if (!mle_.eval<bool>(r)) {
            return false;
        }
    }
    return true;
}

KeyBindingsLogic::KeyBindingsLogic(
    std::string debug_hint,
    std::string section,
    const KeyDescriptions& key_descriptions,
    KeyConfigurations& key_configurations,
    std::string charset,
    std::string ttf_filename,
    std::unique_ptr<IWidget>&& widget,
    const FixedArray<float, 3>& font_color,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    FocusFilter focus_filter,
    MacroLineExecutor mle,
    ButtonStates& button_states,
    std::atomic_size_t& selection_index)
    : globals_changed_{ true }
    , charset_{ std::move(charset) }
    , mle_{ std::move(mle) }
    , key_descriptions_{ key_descriptions }
    , key_configurations_{ key_configurations }
    , contents_{ std::move(section), key_descriptions_, mle_ }
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
        ListViewOrientation::VERTICAL}
{
    mle_.add_observer([this](){
        list_view_.notify_change_visibility();
        globals_changed_ = true;
    });
}

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
    auto s = mle_.eval<std::string>("%input_type");
    auto filter = input_type_from_string(s);
    if (globals_changed_) {
        renderable_text_->set_charset(VariableAndHash{mle_.eval<std::string>(charset_)});
    }
    ListViewStringDrawer drawer{
        ListViewOrientation::VERTICAL,
        *renderable_text_,
        font_height_,
        line_distance_,
        *ew,
        ly,
        [this, filter](size_t index)
        {
            const auto& d = key_descriptions_[index];
            const auto& k = key_configurations_.get(d.id);
            return d.title + std::string(sub_sat<size_t>(40, nchars32(d.title)), ' ') + ": " + k.to_string(filter);
            // std::basic_stringstream<char32_t> sstr;
            // sstr << std::left << std::setw(40) << u8_to_u32_string(d.title);
            // return u32_to_u8_string(sstr.str()) + ": " + k.to_string(filter);
        }};
    list_view_.render_and_handle_input(lx, ly, drawer);
    drawer.render();
    globals_changed_ = false;
}

FocusFilter KeyBindingsLogic::focus_filter() const {
    return focus_filter_;
}

void KeyBindingsLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "KeyBindingsLogic\n";
}
