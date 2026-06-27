#include "Incremental_Versions.hpp"
#include <ostream>

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const IncrementalVersionsRead& versions) {
    ostr << "remote " << versions.local_remote_version <<
        ", base " << versions.remote_base_version <<
        ", new " << versions.remote_new_version;
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const IncrementalVersionsWrite& versions) {
    ostr << "remote " << versions.remote_local_version <<
        ", base " << versions.local_base_version <<
        ", new " << versions.local_new_version;
    return ostr;
}
