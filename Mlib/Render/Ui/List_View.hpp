#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace Mlib {

class TextResource;
class ButtonPress;
class IListViewContents;
class IListViewDrawer;
enum class ListViewOrientation;

class ListView {
public:
    ListView(
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        size_t max_entry_distance,
        const IListViewContents& contents,
        ListViewOrientation orientation,
        const std::function<void()>& on_first_render = std::function<void()>(),
        const std::function<void()>& on_change = std::function<void()>());
    ~ListView();
    void handle_input();
    void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        IListViewDrawer& drawer);
    bool has_selected_element() const;
    size_t selected_element() const;

private:
    std::atomic_size_t& selection_index_;
    size_t max_entry_distance_;
    const IListViewContents& contents_;
    ButtonPress& button_press_;
    std::function<void()> on_first_render_;
    const std::function<void()> on_change_;
    ListViewOrientation orientation_;
};

}
