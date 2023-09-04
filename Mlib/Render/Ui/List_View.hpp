#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
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
    void render_and_handle_input(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        IListViewDrawer& drawer);
    void notify_change_visibility();
    bool has_selected_element() const;
    size_t selected_element() const;

private:
    std::pair<size_t, size_t> render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        IListViewDrawer& drawer);
    void handle_input(size_t left, size_t right);
    std::atomic_size_t& selection_index_;
    const IListViewContents& contents_;
    ButtonPress& button_press_;
    const std::function<void()> on_change_;
    BaseKeyCombination previous_;
    BaseKeyCombination next_;
    BaseKeyCombination previous_fast_;
    BaseKeyCombination next_fast_;
    BaseKeyCombination first_;
    BaseKeyCombination last_;
};

}
