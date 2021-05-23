#pragma once
#include <functional>
#include <memory>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class TextResource;
class ButtonPress;

enum class ListViewOrientation {
    HORIZONTAL,
    VERTICAL
};

template <class TOption>
class ListView {
public:
    ListView(
        ButtonPress& button_press,
        size_t& selection_index,
        const std::string& title,
        const std::vector<TOption>& options,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        ListViewOrientation orientation,
        const std::function<std::string(const TOption&)>& transformation = [](const TOption& s) -> std::string {return s;},
        const std::function<void()>& on_change = [](){});
    ~ListView();
    void handle_input();
    void render(int width, int height, bool periodic_position);
    bool has_selected_element() const;
    const TOption& selected_element() const;

private:
    std::unique_ptr<TextResource> renderable_text_;
    std::string title_;
    const std::vector<TOption>& options_;
    FixedArray<float, 2> position_;
    float line_distance_pixels_;
    std::function<std::string(TOption)> transformation_;
    size_t& selection_index_;
    ButtonPress& button_press_;
    const std::function<void()> on_change_;
    ListViewOrientation orientation_;
};

}

#include "List_View.impl.hpp"
