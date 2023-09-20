#include "Realtime_Dependent_Fps.hpp"

using namespace Mlib;

RealtimeDependentFps::RealtimeDependentFps(std::string prefix,
                                           float dt,
                                           float max_residual_time,
                                           bool control_fps,
                                           bool print_residual_time,
                                           float alpha,
                                           unsigned int print_interval)
    : rts{std::move(prefix), dt, max_residual_time, control_fps, print_residual_time}
    , sms{alpha, print_interval}
    , sls{{&rts, &ds, &sms}}
    , set_fps{sls} {
}
