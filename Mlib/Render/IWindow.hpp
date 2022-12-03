#pragma once

namespace Mlib {

class IWindow {
public:
    virtual void make_current() const = 0;
    virtual void unmake_current() const = 0;
    virtual bool is_initialized() const = 0;
};

}
