#pragma once
#include <Mlib/Time/Fps/Dependent_Sleeper.hpp>
#include <Mlib/Time/Fps/Frame_Time.hpp>
#include <Mlib/Time/Fps/Measure_Fps.hpp>
#include <Mlib/Time/Fps/Realtime_Sleeper.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <Mlib/Time/Fps/Sleeper_Sequence.hpp>

namespace Mlib {

struct RealtimeDependentFps {
    explicit RealtimeDependentFps(
        std::string prefix,
        float dt,
        std::chrono::steady_clock::duration delay,
        float max_residual_time,
        bool control_fps,
        bool print_residual_time,
        float alpha0,
        float alpha1,
        unsigned int print_interval);
    RealtimeSleeper rts;
    DependentSleeper ds;
    MeasureFps mf;
    FrameTime ft;
    SleeperSequence sls;
    SetFps set_fps;
};

}
