#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class AggregateMode {
    OFF = 1 << 0,
    ONCE = 1 << 1,
    SORTED_CONTINUOUSLY = 1 << 2,
    INSTANCES_ONCE = 1 << 3,
    INSTANCES_SORTED_CONTINUOUSLY = 1 << 4
};

inline unsigned int operator | (AggregateMode a, AggregateMode b) {
    return (unsigned int)a | (unsigned int)b;
}

inline unsigned int operator & (AggregateMode a, unsigned int b) {
    return (unsigned int)a & b;
}

inline AggregateMode aggregate_mode_from_string(const std::string& str) {
    if (str == "off") {
        return AggregateMode::OFF;
    } else if (str == "once") {
        return AggregateMode::ONCE;
    } else if (str == "sorted") {
        return AggregateMode::SORTED_CONTINUOUSLY;
    } else if (str == "instances_once") {
        return AggregateMode::INSTANCES_ONCE;
    } else if (str == "instances_sorted") {
        return AggregateMode::INSTANCES_SORTED_CONTINUOUSLY;
    }
    throw std::runtime_error("Unknown aggregate mode");
}

inline std::string aggregate_mode_to_string(AggregateMode aggregate_mode) {
    switch(aggregate_mode) {
    case AggregateMode::OFF:
        return "off";
    case AggregateMode::ONCE:
        return "once";
    case AggregateMode::SORTED_CONTINUOUSLY:
        return "sorted";
    case AggregateMode::INSTANCES_ONCE:
        return "instances_once";
    case AggregateMode::INSTANCES_SORTED_CONTINUOUSLY:
        return "instances_sorted";
    default:
        throw std::runtime_error("Unknown aggregate mode");
    }
}

}
