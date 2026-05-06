#include "Control_Source.hpp"
#include <stdexcept>

using namespace Mlib;

ControlSource Mlib::control_source_from_string(const std::string& control_source) {
    if (control_source == "ai") {
        return ControlSource::AI;
    } else if (control_source == "user") {
        return ControlSource::USER;
    } else {
        throw std::runtime_error("Unknown control source: " + control_source);
    }
}
