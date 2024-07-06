#include "Aggregate_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

AggregateMode Mlib::aggregate_mode_from_string(const std::string& str) {
    if (str == "none") {
        return AggregateMode::NONE;
    } else if (str == "once") {
        return AggregateMode::ONCE;
    } else if (str == "sorted") {
        return AggregateMode::SORTED_CONTINUOUSLY;
    } else if (str == "instances_once") {
        return AggregateMode::INSTANCES_ONCE;
    } else if (str == "instances_sorted") {
        return AggregateMode::INSTANCES_SORTED_CONTINUOUSLY;
    }
    THROW_OR_ABORT("Unknown aggregate mode: \"" + str + '"');
}

std::string Mlib::aggregate_mode_to_string(AggregateMode aggregate_mode) {
    switch(aggregate_mode) {
    case AggregateMode::NONE:
        return "none";
    case AggregateMode::ONCE:
        return "once";
    case AggregateMode::SORTED_CONTINUOUSLY:
        return "sorted";
    case AggregateMode::INSTANCES_ONCE:
        return "instances_once";
    case AggregateMode::INSTANCES_SORTED_CONTINUOUSLY:
        return "instances_sorted";
    default:
        THROW_OR_ABORT("Unknown aggregate mode: \"" + std::to_string((int)aggregate_mode) + '"');
    }
}
