#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <chrono>
#include <functional>

namespace Mlib {

class FrameTime: public ISleeper {
public:
    FrameTime(
        std::chrono::steady_clock::duration delay,
        std::function<std::chrono::steady_clock::duration()> dt,
        float alpha);
    ~FrameTime();

    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;

    std::chrono::steady_clock::time_point frame_time() const;
private:
    std::chrono::steady_clock::duration delay_;
    std::chrono::steady_clock::time_point smooth_time_;
    std::function<std::chrono::steady_clock::duration()> dt_;
    float alpha_;
};

}
