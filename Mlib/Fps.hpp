#pragma once
#include <chrono>

namespace Mlib {

class Fps {
public:
    Fps();
    void tick();
    float last_fps() const;
    float mean_fps() const;
    float mad_fps() const;
private:
    std::chrono::steady_clock::time_point last_time_;
    float last_fps_;
    float mean_fps_;
    float mad_fps_;
};

}
