#include "Incremental_Versions.hpp"
#include <ostream>

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const IncrementalVersionsRead& versions) {
    ostr << "remote " << (versions.local_remote_version + 0) <<
        ", base " << (versions.remote_base_version + 0) <<
        ", new " << (versions.remote_new_version + 0);
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const IncrementalVersionsWrite& versions) {
    ostr << "remote " << (versions.remote_local_version + 0) <<
        ", base " << (versions.local_base_version + 0) <<
        ", new " << (versions.local_new_version + 0);
    return ostr;
}
