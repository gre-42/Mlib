#pragma once
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>

namespace Mlib {

class RealtimeSleeper: public ISleeper {
public:
    explicit RealtimeSleeper(
        std::string prefix,
        float dt,
        float max_residual_time,
        bool print_residual_time);
    ~RealtimeSleeper();
    virtual void tick() override;
    virtual void reset() override;
    virtual bool is_up_to_date() const override;

    void set_dt(float dt);
    std::chrono::steady_clock::time_point simulated_time() const;
private:
    float dt_;
    float max_residual_time_;
    bool print_residual_time_;
    std::chrono::steady_clock::time_point sim_time_;
    std::string prefix_;
    std::atomic_bool is_up_to_date_;
};

}
