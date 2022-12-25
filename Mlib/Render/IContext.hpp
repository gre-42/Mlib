#pragma once

namespace Mlib {

class IContext {
public:
    virtual bool is_initialized() const = 0;
};

}
