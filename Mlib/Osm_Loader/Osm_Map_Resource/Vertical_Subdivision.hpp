#pragma once
#include <string>

namespace Mlib {

enum class VerticalSubdivision {
    NONE = 0,
    SOCLE = (1 << 0),
    ENTRANCES = (1 << 1),

    ANY_SOCLE = SOCLE,
    ANY_ENTRANCES = ENTRANCES
};

inline VerticalSubdivision operator & (VerticalSubdivision a, VerticalSubdivision b) {
    return (VerticalSubdivision)((int)a & (int)b);
}

inline bool any(VerticalSubdivision a) {
    return a != VerticalSubdivision::NONE;
}

inline VerticalSubdivision& operator |= (VerticalSubdivision& a, VerticalSubdivision b) {
    (int&)a |= (int)b;
    return a;
}

VerticalSubdivision vertical_subdivision_from_string(const std::string& s);

}
