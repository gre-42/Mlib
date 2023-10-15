#include "Realtime_Dependent_Fps.hpp"

using namespace Mlib;

RealtimeDependentFps::RealtimeDependentFps(std::string prefix,
                                           float dt,
                                           float max_residual_time,
                                           bool control_fps,
                                           bool print_residual_time,
                                           float alpha,
                                           unsigned int print_interval)
    : rts{std::move(prefix), dt, max_residual_time, print_residual_time}
    , mf{alpha, print_interval}
    , sls{control_fps
        ? std::vector<ISleeper*>{&rts, &ds, &mf}
        : std::vector<ISleeper*>{&mf}}
    , set_fps{&sls}
{}
