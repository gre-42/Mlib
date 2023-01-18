#include "List_View_Viewport_Drawer.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ListViewViewportDrawer::ListViewViewportDrawer(
    const std::function<void(int width, int height)>& draw_left_dots,
    const std::function<void(int width, int height, size_t filtered_index)>& draw_right_dots,
    const std::function<void(int width, int height, size_t index, size_t filtered_index, bool is_selected)>& draw,
    ListViewOrientation orientation,
    float total_length,
    float margin,
    const IEvaluatedWidget& ew,
    const std::vector<SubmenuHeader>& headers)
: draw_left_dots_{draw_left_dots},
  draw_right_dots_{draw_right_dots},
  draw_{draw},
  orientation_{orientation},
  total_length_{total_length},
  margin_{margin},
  ew_{ew},
  headers_{headers}
{}

size_t ListViewViewportDrawer::max_entries_visible() const {
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        return (size_t)((total_length_ + margin_) / (ew_.width() + margin_));
    }
    if (orientation_ == ListViewOrientation::VERTICAL) {
        return (size_t)((total_length_ + margin_) / (ew_.height() + margin_));
    }
    THROW_OR_ABORT("Unknown layout orientation");
}

void ListViewViewportDrawer::draw_left_dots() {
    auto vp = ViewportGuard::from_widget(ew_);
    if (vp.has_value()) {
        draw_left_dots_(vp.value().iwidth(), vp.value().iheight());
    }
}

void ListViewViewportDrawer::draw_right_dots(size_t filtered_index) {
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        auto vp = ViewportGuard::from_widget(EvaluatedWidget::transformed(ew_, (ew_.width() + margin_) * filtered_index, 0.f));
        if (vp.has_value()) {
            draw_right_dots_(vp.value().iwidth(), vp.value().iheight(), filtered_index);
        }
    } else if (orientation_ == ListViewOrientation::VERTICAL) {
        auto vp = ViewportGuard::from_widget(EvaluatedWidget::transformed(ew_, 0.f, (ew_.height() + margin_) * filtered_index));
        if (vp.has_value()) {
            draw_right_dots_(vp.value().iwidth(), vp.value().iheight(), filtered_index);
        }
    } else {
        THROW_OR_ABORT("Unknown layout orientation");
    }
}

void ListViewViewportDrawer::draw_entry(
    size_t index,
    size_t filtered_index,
    bool is_selected,
    bool is_first)
{
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        auto vp = ViewportGuard::from_widget(EvaluatedWidget::transformed(ew_, (ew_.width() + margin_) * filtered_index, 0.f));
        if (vp.has_value()) {
            draw_(vp.value().iwidth(), vp.value().iheight(), index, filtered_index, is_selected);
        }
    } else if (orientation_ == ListViewOrientation::VERTICAL) {
        auto vp = ViewportGuard::from_widget(EvaluatedWidget::transformed(ew_, 0.f, (ew_.height() + margin_) * filtered_index));
        if (vp.has_value()) {
            draw_(vp.value().iwidth(), vp.value().iheight(), index, filtered_index, is_selected);
        }
    } else {
        THROW_OR_ABORT("Unknown layout orientation");
    }
}
