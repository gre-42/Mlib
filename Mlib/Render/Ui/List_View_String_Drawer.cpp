#include "List_View_String_Drawer.hpp"
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Render/Text/Align_Text.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Render/Ui/List_View_Orientation.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ListViewStringDrawer::ListViewStringDrawer(
    ListViewOrientation orientation,
    TextResource& renderable_text,
    const ILayoutPixels& font_height,
    const ILayoutPixels& line_distance,
    const IPixelRegion& ew,
    const LayoutConstraintParameters& ly,
    std::function<std::string(size_t)> transformation)
: orientation_{orientation},
  transformation_{std::move(transformation)},
  font_height_{font_height},
  line_distance_{line_distance},
  renderable_text_{renderable_text},
  ew_{ew},
  ly_{ly}
{
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(_MSC_VER)
    #pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    switch (orientation_) {
        case ListViewOrientation::HORIZONTAL:
            delimiter_ = ""; // sel_left/sel_right already provide spacing
            sel_left_ = "> ";
            sel_right_ = " <";
            break;
        case ListViewOrientation::VERTICAL:
            delimiter_ = '\n';
            sel_left_ = "> ";
            sel_right_ = " <";
            break;
        default:
            THROW_OR_ABORT("Unknown listview orientation");
    }
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif
}

size_t ListViewStringDrawer::max_entries_visible() const {
    float line_distance_pixels = line_distance_.to_pixels(ly_, PixelsRoundMode::NONE);
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        return SIZE_MAX;
    } else {
        return (size_t)std::floor(std::max(0.f, ew_.height()) / line_distance_pixels);
    }
}

void ListViewStringDrawer::draw_left_dots() {
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        sstr_ << "...";
    } else {
        sstr_ << std::string(sel_left_.length(), ' ') << "..." << delimiter_;
    }
}

void ListViewStringDrawer::draw_right_dots(size_t filtered_index) {
    if (orientation_ == ListViewOrientation::HORIZONTAL) {
        sstr_ << delimiter_ << "...";
    } else {
        sstr_ << delimiter_ << std::string(sel_left_.length(), ' ') << "...";
    }
}

void ListViewStringDrawer::draw_entry(
    size_t index,
    size_t filtered_index,
    bool is_selected,
    bool is_first)
{
    if (is_selected) {
        sstr_ << (is_first ? "" : delimiter_) << sel_left_ << transformation_(index) << sel_right_;
    } else {
        sstr_ << (is_first ? "" : delimiter_) << std::string(sel_left_.length(), ' ')
             << transformation_(index);
        if (orientation_ == ListViewOrientation::HORIZONTAL) {
            sstr_ << std::string(sel_right_.length(), ' ');
        }
    }
}

void ListViewStringDrawer::render() {
    renderable_text_.render(
        font_height_.to_pixels(ly_, PixelsRoundMode::ROUND),
        ew_,
        sstr_.str(),
        line_distance_.to_pixels(ly_, PixelsRoundMode::NONE),
        TextInterpolationMode::NEAREST_NEIGHBOR,
        GenericTextAlignment::DEFAULT,
        GenericTextAlignment::DEFAULT);
}
