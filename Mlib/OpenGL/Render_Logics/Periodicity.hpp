#pragma once
#include <string>

namespace Mlib {

enum class Periodicity {
    PERIODIC,
    APERIODIC
};

Periodicity periodicity_from_string(const std::string& s);

}
