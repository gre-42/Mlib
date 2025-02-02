#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

enum class PacenoteDirection {
    LEFT,
    RIGHT
};

PacenoteDirection pacenote_direction_from_string(const std::string& s);

struct Pacenote {
    size_t i0;
    size_t i1;
    double meters_to_start0;
    double meters_to_start1;
    PacenoteDirection direction;
    unsigned int gear;
};

std::ostream& operator << (std::ostream& ostr, PacenoteDirection direction);
std::ostream& operator << (std::ostream& ostr, const Pacenote& pacenote);

}
