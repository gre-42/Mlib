#pragma once
#include <poly2tri/poly2tri.h>

using namespace Mlib;

namespace Mlib {

class PTri {
public:
    p2t::Point* operator () (size_t i) const {
        return v->GetPoint((int)i);
    }
private:
    p2t::Triangle* v;
};

inline std::ostream& operator << (std::ostream& ostr, const p2t::Point& p) {
    ostr << p.x << " " << p.y;
    return ostr;
}

}
