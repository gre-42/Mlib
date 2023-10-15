#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <Mlib/Time/Fps/Measure_Fps.hpp>
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
    MeasureFps measure_fps_;
};

}
