#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>

namespace Mlib {

class SleeperSequence: public ISleeper {
public:
    SleeperSequence(ISleeper& a, ISleeper& b);
    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;
private:
    ISleeper& a_;
    ISleeper& b_;
};

}
