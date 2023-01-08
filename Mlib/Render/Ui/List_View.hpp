#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace Mlib {

class TextResource;
class ButtonPress;
enum class ScreenUnits;

enum class ListViewOrientation {
    HORIZONTAL,
    VERTICAL
};

template <class TOption>
class ListView {
public:
    ListView(
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        const std::string& title,
        const std::vector<TOption>& options,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        float font_height,
        float line_distance,
        ScreenUnits units,
        ListViewOrientation orientation,
        const std::function<std::string(const TOption&)>& transformation = [](const TOption& s) -> std::string {return s;},
        const std::function<void()>& on_first_render = std::function<void()>(),
        const std::function<void()>& on_change = std::function<void()>(),
        const std::function<bool(size_t)>& is_visible = [](size_t selection_index){return true;});
    ~ListView();
    void handle_input();
    void render(int width, int height, float xdpi, float ydpi);
    bool has_selected_element() const;
    const TOption& selected_element() const;

private:
    std::unique_ptr<TextResource> renderable_text_;
    std::string title_;
    const std::vector<TOption>& options_;
    FixedArray<float, 2> position_;
    FixedArray<float, 2> size_;
    float line_distance_;
    std::function<std::string(TOption)> transformation_;
    std::atomic_size_t& selection_index_;
    ButtonPress& button_press_;
    std::function<void()> on_first_render_;
    const std::function<void()> on_change_;
    const std::function<bool(size_t)> is_visible_;
    ListViewOrientation orientation_;
};

}

#include "List_View.impl.hpp"
