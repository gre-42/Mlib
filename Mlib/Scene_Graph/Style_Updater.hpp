#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Style;

class StyleUpdater {
public:
    virtual void notify_movement_intent() = 0;
    virtual void update_style(Style* style) = 0;
};

}
