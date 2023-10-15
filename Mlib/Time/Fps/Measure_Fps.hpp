#pragma once
#include <chrono>

namespace Mlib {

class MeasureFps {
public:
    MeasureFps(float alpha = 0.005f, unsigned int print_interval = UINT_MAX);
    void tick();
    void reset();
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
