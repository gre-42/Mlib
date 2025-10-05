#include "List_View.hpp"
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Make_Key_Binding.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/IList_View_Drawer.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

static const auto& g = make_gamepad_button;
static const auto& a = make_analog_digital_axes;

ListView::ListView(
    std::string debug_hint,
    ButtonStates& button_states,
    size_t selection_index,
    const IListViewContents& contents,
    ListViewOrientation orientation,
    UiFocus& ui_focus,
    uint32_t user_id,
    std::function<void()> on_change)
    : debug_hint_{ std::move(debug_hint) }
    , selection_index_{ selection_index }
    , contents_{ contents }
    , ui_focus_{ ui_focus }
    , on_change_{ std::move(on_change) }
    , previous_{ button_states, key_configurations_, 0, orientation == ListViewOrientation::HORIZONTAL ? "left" : "up", "" }
    , next_{ button_states, key_configurations_, 0, orientation == ListViewOrientation::HORIZONTAL ? "right" : "down", "" }
    , previous_fast_{ button_states, key_configurations_, 0, orientation == ListViewOrientation::HORIZONTAL ? "" : "page_up", "" }
    , next_fast_{ button_states, key_configurations_, 0, orientation == ListViewOrientation::HORIZONTAL ? "" : "page_down", "" }
    , first_{ button_states, key_configurations_, 0, orientation == ListViewOrientation::HORIZONTAL ? "" : "home", "" }
    , last_{ button_states, key_configurations_, 0, orientation == ListViewOrientation::HORIZONTAL ? "" : "end", "" }
{
    AnalogDigitalAxis left{
        .gamepad_id = user_id,
        .axis = 1,
        .sign_and_threshold = -0.5
    };
    AnalogDigitalAxis right{
        .gamepad_id = user_id,
        .axis = 1,
        .sign_and_threshold = 0.5
    };
    AnalogDigitalAxis up{
        .gamepad_id = user_id,
        .axis = 2,
        .sign_and_threshold = -0.5
    };
    AnalogDigitalAxis down{
        .gamepad_id = user_id,
        .axis = 2,
        .sign_and_threshold = 0.5
    };
    auto lock = key_configurations_.lock_exclusive_for(std::chrono::seconds(2), "Key configurations");
    if (user_id == 0) {
        lock->insert(0, "left", { { {{.key = "LEFT", .joystick_axes = a(left, left), .tap_button = g(0, "LEFT")}} } });
        lock->insert(0, "right", { { {{.key = "RIGHT", .joystick_axes = a(right, right), .tap_button = g(0, "RIGHT")}} } });
        lock->insert(0, "up", { {.key_bindings = {{.key = "UP", .joystick_axes = a(up, up), .tap_button = g(0, "UP")}}, .not_key_binding = BaseKeyBinding{.key = "LEFT_CONTROL"}} });
        lock->insert(0, "down", { {.key_bindings = {{.key = "DOWN", .joystick_axes = a(down, down), .tap_button = g(0, "DOWN")}}, .not_key_binding = BaseKeyBinding{.key = "LEFT_CONTROL"}} });
        lock->insert(0, "page_up", { {.key_bindings = {{.key = "PAGE_UP"}}, .not_key_binding = BaseKeyBinding{.key = "LEFT_CONTROL"}} });
        lock->insert(0, "page_down", { {.key_bindings = {{.key = "PAGE_DOWN"}}, .not_key_binding = BaseKeyBinding{.key = "LEFT_CONTROL"}} });
        lock->insert(0, "home", { { {{.key = "HOME"}} } });
        lock->insert(0, "end", { { {{.key = "END"}} } });
    } else {
        lock->insert(0, "left", { { {{.joystick_axes = a(left, left), .tap_button = g(user_id, "LEFT")}} } });
        lock->insert(0, "right", { { {{.joystick_axes = a(right, right), .tap_button = g(user_id, "RIGHT")}} } });
        lock->insert(0, "up", { {.key_bindings = {{.joystick_axes = a(up, up), .tap_button = g(user_id, "UP")}}} });
        lock->insert(0, "down", { {.key_bindings = {{.joystick_axes = a(down, down), .tap_button = g(user_id, "DOWN")}}} });
    }
    if (on_change_ && has_selected_element()) {
        trigger_on_change();
    } else {
        notify_change_visibility();
    }
}

ListView::~ListView() = default;

void ListView::handle_input(size_t left, size_t right) {
    if (contents_.num_entries() == 0) {
        return;
    }
    auto go_to_previous = [this](){
        if (selection_index_ == 0) {
            return;
        }
        size_t new_selection_index = selection_index_ - 1;
        while (true) {
            if (contents_.is_visible(new_selection_index)) {
                selection_index_ = new_selection_index;
                return;
            }
            if (new_selection_index > 0) {
                --new_selection_index;
            } else {
                break;
            }
        }
    };
    auto go_to_next = [this](){
        if (contents_.num_entries() == 0) {
            return;
        }
        if (selection_index_ >= contents_.num_entries() - 1) {
            return;
        }
        size_t new_selection_index = selection_index_ + 1;
        while (true) {
            if (contents_.is_visible(new_selection_index)) {
                selection_index_ = new_selection_index;
                return;
            }
            if (new_selection_index < contents_.num_entries() - 1) {
                ++new_selection_index;
            } else {
                break;
            }
        }
    };
    auto go_to_previous_fast = [this, &go_to_previous, left](){
        if (selection_index_ == left) {
            go_to_previous();
        } else {
            selection_index_ = left;
        }
    };
    auto go_to_next_fast = [this, &go_to_next, right](){
        if (selection_index_ == right) {
            go_to_next();
        } else {
            selection_index_ = right;
        }
    };
    auto go_to_first = [this, &go_to_next](){
        selection_index_ = 0;
        if (!contents_.is_visible(selection_index_)) {
            go_to_next();
        }
    };
    auto go_to_last = [this, &go_to_previous](){
        selection_index_ = contents_.num_entries() - 1;
        if (!contents_.is_visible(selection_index_)) {
            go_to_previous();
        }
    };
    size_t old_selection_index = selection_index_;
    if (previous_.keys_pressed() && !ui_focus_.editing()) {
        go_to_previous();
    }
    if (next_.keys_pressed() && !ui_focus_.editing()) {
        go_to_next();
    }
    if (previous_fast_.keys_pressed() && !ui_focus_.editing()) {
        go_to_previous_fast();
    }
    if (next_fast_.keys_pressed() && !ui_focus_.editing()) {
        go_to_next_fast();
    }
    if (first_.keys_pressed() && !ui_focus_.editing()) {
        go_to_first();
    }
    if (last_.keys_pressed() && !ui_focus_.editing()) {
        go_to_last();
    }
    if (selection_index_ != old_selection_index) {
        trigger_on_change();
    }
}

static std::pair<size_t, size_t> get_visible_window(
    size_t nelements,
    size_t selected_index,
    size_t max_elements_visible)
{
    if ((nelements == 0) ||
        (max_elements_visible == 0))
    {
        return {selected_index, selected_index};
    }
    size_t dleft = (max_elements_visible - 1) / 2;
    size_t dright = max_elements_visible - dleft - 1;
    size_t right = selected_index + dright;
    // Right border exceeded
    if (right >= nelements) {
        // This is the same as max(0, nelements - 1 - (max_elements_visible - 1)),
        // but it avoids negative numbers.
        size_t left = std::max(max_elements_visible, nelements) - max_elements_visible;
        return {left, nelements - 1};
    }
    // Left border exceeded
    if (selected_index < dleft) {
        size_t right2 = std::min(max_elements_visible, nelements) - 1;
        return {0, right2};
    }
    // No border exceeded
    size_t left = selected_index - dleft;
    return {left, right};
}

void ListView::trigger_on_change() {
    if (on_change_) {
        on_change_();
    }
}

std::pair<size_t, size_t> ListView::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    IListViewDrawer& drawer)
{
    if (!has_selected_element()) {
        return {SIZE_MAX, SIZE_MAX};
    }
    std::vector<size_t> filtered_options;
    size_t filtered_selection_index = SIZE_MAX;
    filtered_options.reserve(contents_.num_entries());
    {
        size_t filtered_i = 0;
        for (size_t i = 0; i < contents_.num_entries(); ++i) {
            if (!contents_.is_visible(i)) {
                continue;
            }
            filtered_options.push_back(i);
            if (i == selection_index_) {
                filtered_selection_index = filtered_i;
            }
            ++filtered_i;
        }
    }
    if (filtered_selection_index == SIZE_MAX) {
        THROW_OR_ABORT(debug_hint_ + ": Unexpected filtered_selection_index");
    }
    auto [left, right] = get_visible_window(
        filtered_options.size(),
        filtered_selection_index,
        drawer.max_entries_visible());
    size_t filtered_index = 0;
    bool right_dots_required = false;
    if (std::min(filtered_options.size(), drawer.max_entries_visible()) >= 3) {
        if (left > 0) {
            drawer.draw_left_dots();
            ++left;
            ++filtered_index;
        }
        right_dots_required = (right > 0) && (right + 1 < filtered_options.size());
        if (right_dots_required) {
            --right;
        }
    }
    for (size_t i = left; i <= right; ++i) {
        drawer.draw_entry(
            filtered_options[i],                // index
            filtered_index,                     // filtered_index
            (i == filtered_selection_index),    // is_selected
            (i == left));                       // is_first
        ++filtered_index;
    }
    if (right_dots_required) {
        drawer.draw_right_dots(filtered_index);
    }
    return {filtered_options[left], filtered_options[right]};
}

void ListView::render_and_handle_input(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    IListViewDrawer& drawer)
{
    auto [left, right] = render(lx, ly, drawer);
    if ((left != SIZE_MAX) && (right != SIZE_MAX)) {
        handle_input(left, right);
    }
}

void ListView::notify_change_visibility() {
    if (!has_selected_element()) {
        for (size_t i = 0; i < contents_.num_entries(); ++i) {
            if (contents_.is_visible(i)) {
                selection_index_ = i;
                trigger_on_change();
                break;
            }
        }
        // if ((contents_.num_entries() != 0) && !has_selected_element()) {
        //     THROW_OR_ABORT(debug_hint_ + ": No listview element is selected");
        // }
    }
}

bool ListView::has_selected_element() const {
    return
        (selection_index_ < contents_.num_entries()) &&
        (contents_.is_visible(selection_index_));
}

size_t ListView::selected_element() const {
    if (!has_selected_element()) {
        THROW_OR_ABORT("No element is selected");
    }
    return selection_index_;
}

}
