#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <vector>

namespace Mlib {

class SleeperSequence: public ISleeper {
public:
    explicit SleeperSequence(std::vector<ISleeper*> sleepers);
    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;
private:
    std::vector<ISleeper*> sleepers_;
};

}
