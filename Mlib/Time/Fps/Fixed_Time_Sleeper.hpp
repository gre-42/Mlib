#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>

namespace Mlib {

class FixedTimeSleeper: public ISleeper {
public:
    explicit FixedTimeSleeper(float dt);
    ~FixedTimeSleeper();
    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;
private:
    float dt_;
};

}
