#include "Source_Location.hpp"
#include <ostream>

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const SourceLocation& loc) {
    ostr << loc.file_name() << ':' << loc.line();
    return ostr;
}
