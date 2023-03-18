#pragma once
#include <cstddef>

namespace Mlib {

struct Pacenote;

struct ActivePacenote {
    const Pacenote* pacenote;
    double distance_in_meters;
    double length_in_meters;
};

}
