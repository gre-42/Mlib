#include "Incremental_Remote_Objects.hpp"
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>

using namespace Mlib;

IncrementalRemoteObjects::IncrementalRemoteObjects()
    : next_object_id_{ 0 }
{}

IncrementalRemoteObjects::~IncrementalRemoteObjects() = default;

void IncrementalRemoteObjects::add_new_object(DanglingBaseClassRef<IIncrementalObject> object)
{
    if (!objects_.emplace(
        next_object_id_,
        std::move(object),
        CURRENT_SOURCE_LOCATION).second)
    {
        verbose_abort("Could not add remote object");
    }
    ++next_object_id_;
}

void IncrementalRemoteObjects::add_existing_object(RemoteObjectId id, DanglingBaseClassRef<IIncrementalObject> object)
{
    if (!objects_.emplace(id, std::move(object), CURRENT_SOURCE_LOCATION).second) {
        verbose_abort("Could not add shared object with ID \"" + std::to_string(id) + '"');
    }
}

DanglingBaseClassPtr<IIncrementalObject> IncrementalRemoteObjects::try_get(RemoteObjectId id) const {
    auto it = objects_.find(id);
    if (it == objects_.end()) {
        return nullptr;
    }
    return it->second.object().ptr();
}

SharedObjects::iterator IncrementalRemoteObjects::begin() {
    return objects_.begin();
}

SharedObjects::iterator IncrementalRemoteObjects::end() {
    return objects_.end();
}

size_t IncrementalRemoteObjects::size() const {
    return objects_.size();
}

void IncrementalRemoteObjects::print(std::ostream& ostr) const {
    ostr << "#objects: " << objects_.size();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const IncrementalRemoteObjects& objects) {
    objects.print(ostr);
    return ostr;
}
