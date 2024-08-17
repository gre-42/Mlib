#pragma once
#include <string>

namespace Mlib {

class IRenderableHider {
public:
    virtual void process_input() = 0;
    virtual bool is_visible(const std::string& name) = 0;
};

}
