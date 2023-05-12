#include "Control_Source.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ControlSource Mlib::control_source_from_string(const std::string& control_source) {
    if (control_source == "ai") {
        return ControlSource::AI;
    } else if (control_source == "user") {
        return ControlSource::USER;
    } else {
        THROW_OR_ABORT("Unknown control source: " + control_source);
    }
}
