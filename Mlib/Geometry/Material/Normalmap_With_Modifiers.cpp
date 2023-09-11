#include "Normalmap_With_Modifiers.hpp"
#include <ostream>

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const NormalmapWithModifiers& t) {
    ostr <<
        "filename: " << t.filename << '\n' <<
        "average: " << t.average << '\n';
    return ostr;
}
