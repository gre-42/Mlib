#include "List_View.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Assert.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TOption>
ListView<TOption>::ListView(
    ButtonPress& button_press,
    std::atomic_size_t& selection_index,
    const std::string& title,
    const std::vector<TOption>& options,
    const std::string& ttf_filename,
    const FixedArray<float, 2>& position,
    const FixedArray<float, 2>& size,
    float font_height_pixels,
    float line_distance_pixels,
    ListViewOrientation orientation,
    const std::function<std::string(const TOption&)>& transformation,
    const std::function<void()>& on_first_render,
    const std::function<void()>& on_change,
    const std::function<bool(size_t)>& is_visible)
: renderable_text_{new TextResource{ttf_filename, font_height_pixels}},
  title_{title},
  options_{options},
  position_{position},
  size_{size},
  line_distance_pixels_{line_distance_pixels},
  transformation_{transformation},
  selection_index_{selection_index},
  button_press_{button_press},
  on_first_render_{on_first_render},
  on_change_{on_change},
  is_visible_{is_visible},
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
                if (is_visible_(new_selection_index)) {
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
            if (options_.empty()) {
                return;
            }
            if (selection_index_ >= options_.size() - 1) {
                return;
            }
            size_t new_selection_index = selection_index_ + 1;
            while (true) {
                if (is_visible_(new_selection_index)) {
                    selection_index_ = new_selection_index;
                    return;
                }
                if (new_selection_index < options_.size() - 1) {
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
                if (!is_visible_(selection_index_)) {
                    go_to_next();
                }
                if ((selection_index_ != old_selection_index) && on_change_) {
                    on_change_();
                }
            }
        }
        if (button_press_.key_pressed(last)) {
            if (selection_index_ != options_.size() - 1) {
                size_t old_selection_index = selection_index_;
                selection_index_ = options_.size() - 1;
                if (!is_visible_(selection_index_)) {
                    go_to_previous();
                }
                if ((selection_index_ != old_selection_index) && on_change_) {
                    on_change_();
                }
            }
        }
    }
}

template <class TOption>
void ListView<TOption>::render(int width, int height)
{
    if (on_first_render_) {
        on_first_render_();
        on_first_render_ = std::function<void()>();
    }
    std::string delimiter;
    std::string sel_left;
    std::string sel_right;
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
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
            THROW_OR_ABORT("Unknown listview orientation");
    }
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif
    std::stringstream sstr;
    if (!title_.empty()) {
        sstr << title_ << delimiter;
        sstr << delimiter;
    }
    size_t i = 0;
    for (const auto& s : options_) {
        if (!is_visible_(i)) {
            // Do nothing
        } else if (i == selection_index_) {
            sstr << (i == 0 ? "" : delimiter) << sel_left << transformation_(s) << sel_right;
        } else {
            sstr << (i == 0 ? "" : delimiter) << std::string(sel_left.length(), ' ') << transformation_(s);
            if (orientation_ == ListViewOrientation::HORIZONTAL) {
                sstr << std::string(sel_right.length(), ' ');
            }
        }
        ++i;
    }
    renderable_text_->render(position_, size_, {width, height}, sstr.str(), line_distance_pixels_);
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
