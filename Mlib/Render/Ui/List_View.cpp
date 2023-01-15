#include "List_View.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
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

void ListView::render(
    int width,
    int height,
    float xdpi,
    float ydpi,
    IListViewDrawer& drawer)
{
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
    size_t corrected_max_entry_distance;
    {
        size_t max_lines = drawer.max_entries_visible();
        size_t max_entry_distance = std::min(
            max_entry_distance_,
            (std::max((size_t)1, max_lines) - 1) / 2);
        corrected_max_entry_distance = max_entry_distance;
    }
    size_t extended_max_entry_distance = std::max((size_t)1, corrected_max_entry_distance) - 1;
    if (filtered_selection_index < corrected_max_entry_distance) {
        extended_max_entry_distance += (corrected_max_entry_distance - filtered_selection_index);
    }
    if (filtered_selection_index != SIZE_MAX) {
        size_t distance_to_end = filtered_options.size() - 1 - filtered_selection_index;
        if (distance_to_end < corrected_max_entry_distance) {
            extended_max_entry_distance += (corrected_max_entry_distance - distance_to_end);
        }
    }
    bool is_first = true;
    bool leading_entries_pending = false;
    for (size_t i = 0; i < filtered_options.size(); ++i) {
        size_t distance = (filtered_selection_index > i)
                          ? filtered_selection_index - i
                          : i - filtered_selection_index;
        if (distance > extended_max_entry_distance) {
            if (i > filtered_selection_index) {
                if (i < filtered_options.size() - 1) {
                    drawer.draw_right_dots();
                    break;
                }
            } else {
                if ((i != 0) || (distance > extended_max_entry_distance + 1)) {
                    leading_entries_pending = true;
                    continue;
                }
            }
        }
        if (leading_entries_pending) {
            leading_entries_pending = false;
            is_first = false;
            drawer.draw_left_dots();
        }
        drawer.draw_entry(i, (i == filtered_selection_index), is_first);
        is_first = false;
    }
}

bool ListView::has_selected_element() const {
    return selection_index_ < contents_.num_entries();
}

size_t ListView::selected_element() const {
    assert_true(has_selected_element());
    return selection_index_;
}

}
