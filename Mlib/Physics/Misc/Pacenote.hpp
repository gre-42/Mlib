#pragma once
#include <iosfwd>

namespace Mlib {

enum class PacenoteDirection {
    LEFT,
    RIGHT
};

PacenoteDirection pacenote_direction_from_string(const std::string& s);

struct Pacenote {
    PacenoteDirection direction;
    unsigned int gear;
};

std::ostream& operator << (std::ostream& ostr, PacenoteDirection direction);
std::ostream& operator << (std::ostream& ostr, const Pacenote& pacenote);

}
