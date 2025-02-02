#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

struct RaceIdentifier {
    std::string level;
    std::string time_of_day;
    std::string restrictions;
    std::string session;
    size_t laps = 0;
    // This flag currently has no effect
    uint64_t milliseconds = 0;
    std::string dirname() const;
};

}
