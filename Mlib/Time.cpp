#include "Time.hpp"
#include <cmath>
#include <sstream>

using namespace Mlib;

std::string Mlib::format_minutes_seconds(float seconds) {
    std::stringstream sstr;
    if (!std::isfinite(seconds)) {
        sstr << seconds;
    } else {
        int minutes = seconds / 60;
        float residual_seconds = seconds - 60 * minutes;
        sstr << minutes << " m, " << residual_seconds << " s" << std::endl;
    }
    return sstr.str();
}
