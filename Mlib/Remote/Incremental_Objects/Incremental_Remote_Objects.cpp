#include "Incremental_Remote_Objects.hpp"
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

IncrementalRemoteObjects::IncrementalRemoteObjects(
    RemoteSiteId site_id)
    : site_id_{ site_id }
    , next_local_object_id_{ 0 }
{}

IncrementalRemoteObjects::~IncrementalRemoteObjects() = default;

RemoteSiteId IncrementalRemoteObjects::site_id() const {
    return site_id_;
}

void IncrementalRemoteObjects::add_local_object(const DanglingBaseClassRef<IIncrementalObject>& object)
{
    if (!local_objects_.emplace(
        next_local_object_id_,
        std::move(object),
        CURRENT_SOURCE_LOCATION).second)
    {
        verbose_abort("Could not add remote object");
    }
    ++next_local_object_id_;
}

void IncrementalRemoteObjects::add_remote_object(
    const RemoteObjectId& id,
    const DanglingBaseClassRef<IIncrementalObject>& object)
{
    if (id.site_id == site_id_) {
        THROW_OR_ABORT("Attempt to add a local object as remote");
    }
    if (!remote_objects_.emplace(id, std::move(object), CURRENT_SOURCE_LOCATION).second) {
        THROW_OR_ABORT("Could not add shared object (1): " + id.to_displayname());
    }
}

DanglingBaseClassPtr<IIncrementalObject> IncrementalRemoteObjects::try_get(const RemoteObjectId& id) const {
    if (id.site_id == site_id_) {
        auto it = local_objects_.find(id.object_id);
        if (it == local_objects_.end()) {
            return nullptr;
        }
        return it->second.object().ptr();
    } else {
        auto it = remote_objects_.find(id);
        if (it == remote_objects_.end()) {
            return nullptr;
        }
        return it->second.object().ptr();
    }
}

const LocalObjects& IncrementalRemoteObjects::local_objects() const {
    return local_objects_;
}

const RemoteObjects& IncrementalRemoteObjects::remote_objects() const {
    return remote_objects_;
}

void IncrementalRemoteObjects::print(std::ostream& ostr) const {
    ostr <<
        "#local_objects: " << local_objects_.size() <<
        ", #remote_objects: " << remote_objects_.size();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const IncrementalRemoteObjects& objects) {
    objects.print(ostr);
    return ostr;
}
