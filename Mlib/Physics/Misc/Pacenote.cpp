#include "Pacenote.hpp"
#include <ostream>
#include <stdexcept>

using namespace Mlib;

PacenoteDirection Mlib::pacenote_direction_from_string(const std::string& s) {
    if (s == "left") {
        return PacenoteDirection::LEFT;
    }
    if (s == "right") {
        return PacenoteDirection::RIGHT;
    }
    throw std::runtime_error("Unknown pacenote direction: \"" + s + '"');
}

std::ostream& Mlib::operator << (std::ostream& ostr, PacenoteDirection direction) {
    if (direction == PacenoteDirection::LEFT) {
        ostr << "left";
    } else if (direction == PacenoteDirection::RIGHT) {
        ostr << "right";
    } else {
        throw std::runtime_error("Unknown pacenote direction");
    }
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Pacenote& pacenote) {
    ostr << pacenote.gear << " " << pacenote.direction;
    return ostr;
}
