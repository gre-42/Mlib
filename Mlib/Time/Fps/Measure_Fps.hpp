#pragma once
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <chrono>
#include <climits>
#include <cstdint>

namespace Mlib {

class MeasureFps: public ISleeper {
public:
    MeasureFps(double alpha = 0.005, uint32_t print_interval = UINT32_MAX);
    ~MeasureFps();

    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;

    double mean_dt() const;
    double mad_dt() const;

    double mean_fps() const;
    double mad_fps() const;
    double min_fps() const;
    double max_fps() const;
private:
    std::chrono::steady_clock::time_point last_time_;
    ExponentialSmoother<double> mean_dt_;
    ExponentialSmoother<double> mad_dt_;
    double alpha_;
    uint32_t print_counter_;
    uint32_t print_interval_;
};

}
