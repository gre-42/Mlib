#pragma once
#include <string>

namespace Mlib {

class IWindow {
public:
    virtual void set_title(const std::string& title) = 0;
    virtual void make_current() const = 0;
    virtual void unmake_current() const = 0;
};

}
