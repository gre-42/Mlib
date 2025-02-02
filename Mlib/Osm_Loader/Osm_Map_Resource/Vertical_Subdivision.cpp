#include "Vertical_Subdivision.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

VerticalSubdivision Mlib::vertical_subdivision_from_string(const std::string& s) {
    if (s == "no") {
        return VerticalSubdivision::NO;
    } else if (s == "socle") {
        return VerticalSubdivision::SOCLE;
    } else if (s == "entrances") {
        return VerticalSubdivision::ENTRANCES;
    } else {
        THROW_OR_ABORT("Unknown vertical subdivision: \"" + s + '"');
    }
}
