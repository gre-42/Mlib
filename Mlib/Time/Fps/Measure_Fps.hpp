#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <chrono>
#include <climits>

namespace Mlib {

class MeasureFps: public ISleeper {
public:
    MeasureFps(float alpha = 0.005f, unsigned int print_interval = UINT_MAX);
    ~MeasureFps();

    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;

    float mean_dt() const;

    float last_fps() const;
    float mean_fps() const;
    float mad_fps() const;
    float min_fps() const;
    float max_fps() const;
private:
    std::chrono::steady_clock::time_point last_time_;
    float last_dt_;
    float mean_dt_;
    float mad_dt_;
    float alpha_;
    unsigned int print_counter_;
    unsigned int print_interval_;
};

}
