#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <chrono>
#include <climits>

namespace Mlib {

class MeasureFps: public ISleeper {
public:
    MeasureFps(double alpha = 0.005, unsigned int print_interval = UINT_MAX);
    ~MeasureFps();

    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;

    double mean_dt() const;

    double last_fps() const;
    double mean_fps() const;
    double mad_fps() const;
    double min_fps() const;
    double max_fps() const;
private:
    std::chrono::steady_clock::time_point last_time_;
    double last_dt_;
    double mean_dt_;
    double mad_dt_;
    double alpha_;
    unsigned int print_counter_;
    unsigned int print_interval_;
};

}
