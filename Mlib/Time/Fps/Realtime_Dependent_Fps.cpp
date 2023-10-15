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
    , sms{alpha, print_interval}
    , sls{control_fps
        ? std::vector<ISleeper*>{&rts, &ds, &sms}
        : std::vector<ISleeper*>{&sms}}
    , set_fps{&sls}
{}
