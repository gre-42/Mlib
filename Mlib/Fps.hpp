#pragma once
#include <chrono>

namespace Mlib {

class Fps {
public:
    Fps();
    void tick();
    float fps() const;
private:
    std::chrono::steady_clock::time_point last_time_;
    float fps_;
};

}
