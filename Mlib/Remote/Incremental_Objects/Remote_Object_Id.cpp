#include "Remote_Object_Id.hpp"

using namespace Mlib;

std::string RemoteObjectId::to_string() const {
    return std::to_string(site_id) + '_' + std::to_string(object_id);
}

std::string RemoteObjectId::to_displayname() const {
    return "site: " + std::to_string(site_id) + ", object: " + std::to_string(object_id);
}

std::ostream& Mlib::operator << (std::ostream& ostr, RemoteObjectId id) {
    ostr << id.to_displayname();
    return ostr;
}
