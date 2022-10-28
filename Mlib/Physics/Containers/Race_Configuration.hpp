#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

struct RaceConfiguration {
    std::string session;
    size_t laps = 0;
    // This flag currently has no effect
    uint64_t milliseconds = 0;
    // This flag currently has no effect
    bool readonly = false;
    std::string dirname() const;
};

}
