#pragma once
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class IListViewContents{
public:
    virtual size_t num_entries() const = 0;
    virtual bool is_visible(size_t index) const = 0;
};

}
