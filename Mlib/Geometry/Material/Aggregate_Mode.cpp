#include "Aggregate_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

AggregateMode Mlib::aggregate_mode_from_string(const std::string& str) {
    static const std::map<std::string, AggregateMode> m{
        {"none", AggregateMode::NONE},
        {"once", AggregateMode::ONCE},
        {"sorted", AggregateMode::SORTED_CONTINUOUSLY},
        {"instances_once", AggregateMode::INSTANCES_ONCE},
        {"instances_sorted", AggregateMode::INSTANCES_SORTED_CONTINUOUSLY}};
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown aggregate mode: \"" + str + '"');
    }
    return it->second;
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
    }
    THROW_OR_ABORT("Unknown aggregate mode: \"" + std::to_string((int)aggregate_mode) + '"');
}
