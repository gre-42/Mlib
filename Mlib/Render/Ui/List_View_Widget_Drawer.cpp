#include "List_View_Widget_Drawer.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ListViewWidgetDrawer::ListViewWidgetDrawer(
    std::function<void(const IPixelRegion& ew)> draw_left_dots,
    std::function<void(const IPixelRegion& ew)> draw_right_dots,
    std::function<void(float dx, float dy, size_t index, bool is_selected)> draw,
    ListViewOrientation orientation,
    float total_length,
    float margin,
    const IPixelRegion& ew_ref)
    : draw_left_dots_{ std::move(draw_left_dots) }
    , draw_right_dots_{ std::move(draw_right_dots) }
    , draw_{ std::move(draw) }
    , orientation_{ orientation }
    , total_length_{ total_length }
    , margin_{ margin }
    , ew_ref_{ ew_ref }
{}

size_t ListViewWidgetDrawer::max_entries_visible() const {
    // w = n * c + (n-1) * m => n = (w + m) / (c + m)
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        return (size_t)(std::max(0.f, (total_length_ + margin_) / (ew_ref_.width() + margin_)));
    }
    if (orientation_ == ListViewOrientation::VERTICAL) {
        return (size_t)(std::max(0.f, (total_length_ + margin_) / (ew_ref_.height() + margin_)));
    }
    THROW_OR_ABORT("Unknown layout orientation");
}

void ListViewWidgetDrawer::draw_left_dots() {
    draw_left_dots_(ew_ref_);
}

void ListViewWidgetDrawer::draw_right_dots(size_t filtered_index) {
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        auto ew = PixelRegion::transformed(ew_ref_, (ew_ref_.width() + margin_) * float(filtered_index), 0.f);
        draw_right_dots_(ew);
    } else if (orientation_ == ListViewOrientation::VERTICAL) {
        auto ew = PixelRegion::transformed(ew_ref_, 0.f, (ew_ref_.height() + margin_) * float(filtered_index));
        draw_right_dots_(ew);
    } else {
        THROW_OR_ABORT("Unknown layout orientation");
    }
}

void ListViewWidgetDrawer::draw_entry(
    size_t index,
    size_t filtered_index,
    bool is_selected,
    bool is_first)
{
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        draw_((ew_ref_.width() + margin_) * float(filtered_index), 0.f, index, is_selected);
    } else if (orientation_ == ListViewOrientation::VERTICAL) {
        draw_(0.f, (ew_ref_.height() + margin_) * float(filtered_index), index, is_selected);
    } else {
        THROW_OR_ABORT("Unknown layout orientation");
    }
}
