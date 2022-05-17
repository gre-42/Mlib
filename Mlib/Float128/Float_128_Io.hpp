#pragma once
#include "Float_128.hpp"
#include <iosfwd>

namespace std {

std::istream& operator >> (std::istream& istr, Mlib::float128& f);

std::ostream& operator << (std::ostream& ostr, const Mlib::float128& f);

}
