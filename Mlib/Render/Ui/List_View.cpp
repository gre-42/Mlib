#include "List_View.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/IList_View_Drawer.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

ListView::ListView(
    std::string debug_hint,
    ButtonStates& button_states,
    size_t selection_index,
    const IListViewContents& contents,
    ListViewOrientation orientation,
    std::function<void()> on_change)
    : debug_hint_{ std::move(debug_hint) }
    , selection_index_{ selection_index }
    , contents_{ contents }
    , on_change_{ std::move(on_change) }
    , previous_{ button_states, key_configurations_, orientation == ListViewOrientation::HORIZONTAL ? "left" : "up", "" }
    , next_{ button_states, key_configurations_, orientation == ListViewOrientation::HORIZONTAL ? "right" : "down", "" }
    , previous_fast_{ button_states, key_configurations_, orientation == ListViewOrientation::HORIZONTAL ? "" : "page_up", "" }
    , next_fast_{ button_states, key_configurations_, orientation == ListViewOrientation::HORIZONTAL ? "" : "page_down", "" }
    , first_{ button_states, key_configurations_, orientation == ListViewOrientation::HORIZONTAL ? "" : "home", "" }
    , last_{ button_states, key_configurations_, orientation == ListViewOrientation::HORIZONTAL ? "" : "end", "" }
{
    AnalogDigitalAxis left{
            .axis = "1",
            .sign_and_threshold = -0.5
    };
    AnalogDigitalAxis right{
            .axis = "1",
            .sign_and_threshold = 0.5
    };
    AnalogDigitalAxis up{
            .axis = "2",
            .sign_and_threshold = -0.5
    };
    AnalogDigitalAxis down{
            .axis = "2",
            .sign_and_threshold = 0.5
    };
    auto lock = key_configurations_.lock_exclusive_for(std::chrono::seconds(2), "Key configurations");
    auto& cfg = lock->emplace();
    cfg.insert("left", { { {{.key = "LEFT", .joystick_axes = {{"default", {.joystick=left, .tap = left}}}, .tap_button = "LEFT"}} } });
    cfg.insert("right", { { {{.key = "RIGHT", .joystick_axes = {{"default", {.joystick=right, .tap = right}}}, .tap_button = "RIGHT"}} } });
    cfg.insert("up", { {.key_bindings = {{.key = "UP", .joystick_axes = {{"default", {.joystick = up, .tap = up}}}, .tap_button = "UP"}}, .not_key_binding = BaseKeyBinding{.key = "LEFT_CONTROL"}} });
    cfg.insert("down", { {.key_bindings = {{.key = "DOWN", .joystick_axes = {{"default", {.joystick = down, .tap = down}}}, .tap_button = "DOWN"}}, .not_key_binding = BaseKeyBinding{.key = "LEFT_CONTROL"}} });
    cfg.insert("page_up", { {.key_bindings = {{.key = "PAGE_UP"}}, .not_key_binding = BaseKeyBinding{.key = "LEFT_CONTROL"}} });
    cfg.insert("page_down", { {.key_bindings = {{.key = "PAGE_DOWN"}}, .not_key_binding = BaseKeyBinding{.key = "LEFT_CONTROL"}} });
    cfg.insert("home", { { {{.key = "HOME"}} } });
    cfg.insert("end", { { {{.key = "END"}} } });
    if (on_change_ && has_selected_element()) {
        on_change_();
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
    if (previous_.keys_pressed()) {
        go_to_previous();
    }
    if (next_.keys_pressed()) {
        go_to_next();
    }
    if (previous_fast_.keys_pressed()) {
        go_to_previous_fast();
    }
    if (next_fast_.keys_pressed()) {
        go_to_next_fast();
    }
    if (first_.keys_pressed()) {
        go_to_first();
    }
    if (last_.keys_pressed()) {
        go_to_last();
    }
    if ((selection_index_ != old_selection_index) && on_change_) {
        on_change_();
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
                if (on_change_) {
                    on_change_();
                }
                break;
            }
        }
        if ((contents_.num_entries() != 0) && !has_selected_element()) {
            THROW_OR_ABORT(debug_hint_ + ": No listview element is selected");
        }
    }
}

bool ListView::has_selected_element() const {
    return
        (selection_index_ < contents_.num_entries()) &&
        (contents_.is_visible(selection_index_));
}

size_t ListView::selected_element() const {
    assert_true(has_selected_element());
    return selection_index_;
}

}
