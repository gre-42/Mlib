#pragma once
#include <cstddef>

namespace Mlib {

class IListViewDrawer {
public:
    virtual ~IListViewDrawer() = default;
    virtual size_t max_entries_visible() const = 0;
    virtual void draw_left_dots() = 0;
    virtual void draw_right_dots(size_t filtered_index) = 0;
    virtual void draw_entry(
        size_t index,
        size_t filtered_index,
        bool is_selected,
        bool is_first) = 0;
};

}
