#pragma once

namespace Mlib {

class ISleeper {
public:
    virtual ~ISleeper() = default;
    virtual void tick() = 0;
    virtual void reset() = 0;
    virtual bool is_up_to_date() const = 0;
};

}
