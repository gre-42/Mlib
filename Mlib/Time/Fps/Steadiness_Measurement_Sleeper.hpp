#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <chrono>

namespace Mlib {

class SteadinessMeasurementSleeper: public ISleeper {
public:
    SteadinessMeasurementSleeper(float alpha, unsigned int print_interval);
    ~SteadinessMeasurementSleeper();
    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;
private:
    unsigned int print_counter_;
    unsigned int print_interval_;
    std::chrono::steady_clock::time_point time_old_;
    float mean_;
    float deviation_;
    float alpha_;
};

}
