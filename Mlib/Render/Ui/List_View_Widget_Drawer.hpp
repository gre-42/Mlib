#pragma once
#include <Mlib/Render/Ui/IList_View_Drawer.hpp>
#include <functional>
#include <vector>

namespace Mlib {

class ILayoutPixels;
class IPixelRegion;
enum class ListViewOrientation;

class ListViewWidgetDrawer: public IListViewDrawer {
public:
    explicit ListViewWidgetDrawer(
        const std::function<void(const IPixelRegion& ew)>& draw_left_dots,
        const std::function<void(const IPixelRegion& ew)>& draw_right_dots,
        const std::function<void(const IPixelRegion& ew, size_t index, bool is_selected)>& draw,
        ListViewOrientation orientation,
        float total_length,
        float margin,
        const IPixelRegion& ew_ref);
    // IListViewDrawer
    virtual size_t max_entries_visible() const override;
    virtual void draw_left_dots() override;
    virtual void draw_right_dots(size_t filtered_index) override;
    virtual void draw_entry(
        size_t index,
        size_t filtered_index,
        bool is_selected,
        bool is_first) override;

private:
    std::function<void(const IPixelRegion& ew)> draw_left_dots_;
    std::function<void(const IPixelRegion& ew)> draw_right_dots_;
    std::function<void(const IPixelRegion& ew, size_t index, bool is_selected)> draw_;
    ListViewOrientation orientation_;
    float total_length_;
    float margin_;
    const IPixelRegion& ew_ref_;
};

}
