#include "List_View.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

namespace Mlib {

template <class TOption>
ListView<TOption>::ListView(
    ButtonPress& button_press,
    size_t& selection_index,
    const std::string& title,
    const std::vector<TOption>& options,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    float font_height_pixels,
    float line_distance_pixels,
    ListViewOrientation orientation,
    const std::function<std::string(const TOption&)>& transformation,
    const std::function<void()>& on_change)
: renderable_text_{new TextResource{ttf_filename, font_height_pixels}},
  title_{title},
  options_{options},
  position_{position},
  line_distance_pixels_{line_distance_pixels},
  transformation_{transformation},
  selection_index_{selection_index},
  button_press_{button_press},
  on_change_{on_change},
  orientation_{orientation}
{}

template <class TOption>
ListView<TOption>::~ListView()
{}

template <class TOption>
void ListView<TOption>::handle_input() {
    if (!options_.empty()) {
        BaseKeyBinding previous;
        BaseKeyBinding next;
        BaseKeyBinding first;
        BaseKeyBinding last;
        switch (orientation_) {
            case ListViewOrientation::HORIZONTAL:
                previous = {.key = "LEFT", .joystick_axis = "1", .joystick_axis_sign = -1};
                next = {.key = "RIGHT", .joystick_axis = "1", .joystick_axis_sign = 1};
                first = {.key = ""};
                last = {.key = ""};
                break;
            case ListViewOrientation::VERTICAL:
                previous = {.key = "UP", .joystick_axis = "2", .joystick_axis_sign = -1};
                next = {.key = "DOWN", .joystick_axis = "2", .joystick_axis_sign = 1};
                first = {.key = "HOME"};
                last = {.key = "END"};
                break;
            default:
                throw std::runtime_error("Unknown listview orientation");
        }
        if (button_press_.key_pressed(previous)) {
            if (selection_index_ > 0) {
                --selection_index_;
                on_change_();
            }
        }
        if (button_press_.key_pressed(next)) {
            if (selection_index_ < options_.size() - 1) {
                ++selection_index_;
                on_change_();
            }
        }
        if (button_press_.key_pressed(first)) {
            if (selection_index_ != 0) {
                selection_index_ = 0;
                on_change_();
            }
        }
        if (button_press_.key_pressed(last)) {
            if (selection_index_ != options_.size() - 1) {
                selection_index_ = options_.size() - 1;
                on_change_();
            }
        }
    }
}

template <class TOption>
void ListView<TOption>::render(int width, int height, bool periodic_position)
{
    std::string delimiter;
    std::string sel_left;
    std::string sel_right;
    switch (orientation_) {
        case ListViewOrientation::HORIZONTAL:
            delimiter = "    ";
            sel_left = "> ";
            sel_right = " <";
            break;
        case ListViewOrientation::VERTICAL:
            delimiter = '\n';
            sel_left = "> ";
            sel_right = " <";
            break;
        default:
            throw std::runtime_error("Unknown listview orientation");
    }
    std::stringstream sstr;
    if (!title_.empty()) {
        sstr << title_ << delimiter;
        sstr << delimiter;
    }
    size_t i = 0;
    for (const auto& s : options_) {
        if (i == selection_index_) {
            sstr << (i == 0 ? "" : delimiter) << sel_left << transformation_(s) << sel_right;
        } else {
            sstr << (i == 0 ? "" : delimiter) << std::string(sel_left.length(), ' ') << transformation_(s);
            if (orientation_ == ListViewOrientation::HORIZONTAL) {
                sstr << std::string(sel_right.length(), ' ');
            }
        }
        ++i;
    }
    renderable_text_->render(position_, sstr.str(), width, height, line_distance_pixels_, periodic_position);
}

template <class TOption>
bool ListView<TOption>::has_selected_element() const {
    return selection_index_ < options_.size();
}

template <class TOption>
const TOption& ListView<TOption>::selected_element() const {
    assert_true(has_selected_element());
    return options_[selection_index_];
}

}
