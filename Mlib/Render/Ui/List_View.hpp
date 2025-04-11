#pragma once
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace Mlib {

class TextResource;
class IListViewContents;
class IListViewDrawer;
enum class ListViewOrientation;
struct LayoutConstraintParameters;

class ListView {
public:
    ListView(
        std::string debug_hint,
        ButtonStates& button_states,
        size_t selection_index,
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
    std::string debug_hint_;
    size_t selection_index_;
    const IListViewContents& contents_;
    const std::function<void()> on_change_;
    ButtonPress previous_;
    ButtonPress next_;
    ButtonPress previous_fast_;
    ButtonPress next_fast_;
    ButtonPress first_;
    ButtonPress last_;
    LockableKeyConfigurations key_configurations_;
};

}
