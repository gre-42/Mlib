#pragma once
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
struct LayoutConstraintParameters;

class ListView {
public:
    ListView(
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        const IListViewContents& contents,
        ListViewOrientation orientation,
        std::function<void()> on_change = std::function<void()>());
    ~ListView();
    void handle_input();
    void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        IListViewDrawer& drawer);
    void notify_change_visibility();
    bool has_selected_element() const;
    size_t selected_element() const;

private:
    std::atomic_size_t& selection_index_;
    const IListViewContents& contents_;
    ButtonPress& button_press_;
    const std::function<void()> on_change_;
    ListViewOrientation orientation_;
};

}
