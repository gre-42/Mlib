#pragma once
#include <string>

namespace Mlib {

struct BaseRotor {
    explicit BaseRotor(const std::string& engine);
    std::string engine;
    float angular_velocity;
};

}
