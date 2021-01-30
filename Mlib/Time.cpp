#include "Time.hpp"
#include <cmath>
#include <sstream>

using namespace Mlib;

std::string Mlib::format_minutes_seconds(float seconds) {
    std::stringstream sstr;
    if (!std::isfinite(seconds)) {
        sstr << seconds;
    } else {
        unsigned int minutes = (unsigned int)(seconds / 60);
        float residual_seconds = seconds - 60 * minutes;
        sstr << minutes << " m, " << residual_seconds << " s";
    }
    return sstr.str();
}
