#include "Remote_Object_Id.hpp"
#include <sstream>

using namespace Mlib;

std::string RemoteObjectId::to_string() const {
    return (std::stringstream() << (site_id + 0) << '_' << object_id).str();
}

std::string RemoteObjectId::to_displayname() const {
    return (std::stringstream() << "site: " << (site_id + 0) << ", object: " << object_id).str();
}

std::ostream& Mlib::operator << (std::ostream& ostr, RemoteObjectId id) {
    ostr << id.to_displayname();
    return ostr;
}
