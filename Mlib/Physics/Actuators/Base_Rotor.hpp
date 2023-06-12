#pragma once
#include <optional>
#include <string>

namespace Mlib {

struct BaseRotor {
    BaseRotor(
        std::string engine,
        std::optional<std::string> delta_engine);
    std::string engine;
    std::optional<std::string> delta_engine;
    float angular_velocity;
};

}
