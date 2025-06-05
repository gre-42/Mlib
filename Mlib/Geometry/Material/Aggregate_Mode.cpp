#include "Aggregate_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

AggregateMode Mlib::aggregate_mode_from_string(const std::string& str) {
    static const std::map<std::string, AggregateMode> m{
        {"none", AggregateMode::NONE},
        {"once", AggregateMode::ONCE},
        {"sorted", AggregateMode::SORTED_CONTINUOUSLY},
        {"instances_once", AggregateMode::INSTANCES_ONCE},
        {"instances_sorted", AggregateMode::INSTANCES_SORTED_CONTINUOUSLY},
        {"object_mask", AggregateMode::OBJECT_MASK},
        {"instances_mask", AggregateMode::INSTANCES_MASK}};
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown aggregate mode: \"" + str + '"');
    }
    return it->second;
}

std::string Mlib::aggregate_mode_to_string(AggregateMode aggregate_mode) {
    switch (aggregate_mode) {
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
    case AggregateMode::NODE_OBJECT:
        return "node_object";
    case AggregateMode::NODE_TRIANGLES:
        return "node_triangles";
    case AggregateMode::OBJECT_MASK:
        return "object_mask";
    case AggregateMode::INSTANCES_MASK:
        return "instances_mask";
    }
    THROW_OR_ABORT("Unknown aggregate mode: \"" + std::to_string((int)aggregate_mode) + '"');
}
