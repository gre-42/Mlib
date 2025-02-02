#include "Realtime_Dependent_Fps.hpp"

using namespace Mlib;

RealtimeDependentFps::RealtimeDependentFps(
    std::string prefix,
    float dt,
    std::chrono::steady_clock::duration delay,
    float max_residual_time,
    bool control_fps,
    bool print_residual_time,
    float alpha0,
    float alpha1,
    unsigned int print_interval)
    : rts{ std::move(prefix), dt, max_residual_time, print_residual_time }
    , mf{ alpha0, print_interval }
    , ft{
        delay,
        [this]() {
            return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>{ mf.mean_dt() });
        },
        alpha1 }
    , sls{control_fps
        ? std::vector<ISleeper*>{&rts, &ds, &mf}
        : std::vector<ISleeper*>{&mf, &ft}}
    , set_fps{&sls, std::function<std::chrono::steady_clock::time_point()>(), [](){ return false; }}
{}
