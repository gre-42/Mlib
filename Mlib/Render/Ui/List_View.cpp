#include "List_View.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/IList_View_Drawer.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

ListView::ListView(
    ButtonPress& button_press,
    std::atomic_size_t& selection_index,
    size_t max_entry_distance,
    const IListViewContents& contents,
    ListViewOrientation orientation,
    const std::function<void()>& on_first_render,
    const std::function<void()>& on_change)
: selection_index_{selection_index},
  max_entry_distance_{max_entry_distance},
  contents_{contents},
  button_press_{button_press},
  on_first_render_{on_first_render},
  on_change_{on_change},
  orientation_{orientation}
{}

ListView::~ListView() = default;

void ListView::handle_input() {
    if (contents_.num_entries() != 0) {
        BaseKeyBinding previous;
        BaseKeyBinding next;
        BaseKeyBinding first;
        BaseKeyBinding last;
        switch (orientation_) {
            case ListViewOrientation::HORIZONTAL:
                previous = {.key = "LEFT", .joystick_axis = "1", .joystick_axis_sign = -1, .tap_button = "LEFT"};
                next = {.key = "RIGHT", .joystick_axis = "1", .joystick_axis_sign = 1, .tap_button = "RIGHT"};
                first = {.key = ""};
                last = {.key = ""};
                break;
            case ListViewOrientation::VERTICAL:
                previous = {.key = "UP", .joystick_axis = "2", .joystick_axis_sign = -1, .tap_button = "UP"};
                next = {.key = "DOWN", .joystick_axis = "2", .joystick_axis_sign = 1, .tap_button = "DOWN"};
                first = {.key = "HOME"};
                last = {.key = "END"};
                break;
            default:
                THROW_OR_ABORT("Unknown listview orientation");
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
        if (button_press_.key_pressed(previous)) {
            size_t old_selection_index = selection_index_;
            go_to_previous();
            if ((selection_index_ != old_selection_index) && on_change_) {
                on_change_();
            }
        }
        if (button_press_.key_pressed(next)) {
            size_t old_selection_index = selection_index_;
            go_to_next();
            if ((selection_index_ != old_selection_index) && on_change_) {
                on_change_();
            }
        }
        if (button_press_.key_pressed(first)) {
            if (selection_index_ != 0) {
                size_t old_selection_index = selection_index_;
                selection_index_ = 0;
                if (!contents_.is_visible(selection_index_)) {
                    go_to_next();
                }
                if ((selection_index_ != old_selection_index) && on_change_) {
                    on_change_();
                }
            }
        }
        if (button_press_.key_pressed(last)) {
            if (selection_index_ != contents_.num_entries() - 1) {
                size_t old_selection_index = selection_index_;
                selection_index_ = contents_.num_entries() - 1;
                if (!contents_.is_visible(selection_index_)) {
                    go_to_previous();
                }
                if ((selection_index_ != old_selection_index) && on_change_) {
                    on_change_();
                }
            }
        }
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
        size_t right = std::min(max_elements_visible, nelements) - 1;
        return {0, right};
    }
    // No border exceeded
    size_t left = selected_index - dleft;
    return {left, right};
}

void ListView::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    IListViewDrawer& drawer)
{
    if (!has_selected_element()) {
        return;
    }
    if (on_first_render_) {
        on_first_render_();
        on_first_render_ = std::function<void()>();
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
        THROW_OR_ABORT("Unexpected filtered_selection_index");
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
}

void ListView::notify_change_visibility() {
    if (!has_selected_element()) {
        for (size_t i = 0; i < contents_.num_entries(); ++i) {
            if (contents_.is_visible(i)) {
                selection_index_ = i;
                on_change_();
                break;
            }
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
