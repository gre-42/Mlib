#include "Float_128_Io.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>
#include <quadmath.h>
#include <stdexcept>

using namespace Mlib;

std::istream& std::operator >> (std::istream& istr, float128& f) {
    std::string s;
    istr >> s;
    const char* cs = s.c_str();
    char *sp;
    f = strtoflt128(cs, &sp);
    if ((size_t)(sp - cs) != s.length()) {
        THROW_OR_ABORT("Could not parse string as float128: \"" + s + '"');
    }
    return istr;
}

std::ostream& std::operator << (std::ostream& ostr, const float128& f) {
    // From: https://www.boost.org/doc/libs/1_65_0/boost/multiprecision/float128.hpp
    //       int max_digits10 = 36;
    char buf[128];
    int n = quadmath_snprintf (buf, sizeof buf, "%.36Qe", f);
    if (((size_t) n >= sizeof buf) || (n < 0)) {
        THROW_OR_ABORT"Could not convert float128 to string");
    }
    ostr << buf;
    return ostr;
}
