#include "Realtime_Dependent_Fps.hpp"

using namespace Mlib;

RealtimeDependentFps::RealtimeDependentFps(std::string prefix,
                                           float dt,
                                           float physics_dt,
                                           float max_residual_time,
                                           bool control_fps,
                                           bool print_residual_time,
                                           float alpha,
                                           unsigned int print_interval)
    : rts{std::move(prefix), dt, max_residual_time, print_residual_time}
    , mf{alpha, print_interval}
    , ft{
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<float>{ 1.0f * physics_dt }),
        [this]() {
            return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<float>{ mf.mean_dt() });
        },
        0.05f }
    , sls{control_fps
        ? std::vector<ISleeper*>{&rts, &ds, &mf}
        : std::vector<ISleeper*>{&mf, &ft}}
    , set_fps{&sls, std::function<std::chrono::steady_clock::time_point()>(), [](){ return false; }}
{}
