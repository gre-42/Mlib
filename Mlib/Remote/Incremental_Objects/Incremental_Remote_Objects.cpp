#include "Incremental_Remote_Objects.hpp"
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/Scene_Level.hpp>
#include <Mlib/Scene_Config/Remote_Event_History_Duration.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

IncrementalRemoteObjects::IncrementalRemoteObjects(
    RemoteSiteId local_site_id,
    const DanglingBaseClassRef<SceneLevelSelector>& local_scene_level_selector)
    : local_site_id_{ local_site_id }
    , local_scene_level_selector_{ local_scene_level_selector }
    , next_local_object_id_{ 0 }
{}

IncrementalRemoteObjects::~IncrementalRemoteObjects() {
    on_destroy.clear();
}

RemoteSiteId IncrementalRemoteObjects::local_site_id() const {
    return local_site_id_;
}

std::chrono::steady_clock::time_point IncrementalRemoteObjects::local_time() const {
    return local_time_.time();
}

PauseStatus IncrementalRemoteObjects::pause_status() const {
    return local_time_.status();
}

void IncrementalRemoteObjects::set_local_time(
    const TimeAndPause<std::chrono::steady_clock::time_point>& time)
{
    local_time_ = time;
}

DanglingBaseClassRef<SceneLevelSelector> IncrementalRemoteObjects::local_scene_level_selector() const {
    return local_scene_level_selector_;
}

RemoteObjectId IncrementalRemoteObjects::add_local_object(
    const DanglingBaseClassRef<IIncrementalObject>& object,
    RemoteObjectVisibility visibility)
{
    auto& objects = [&]() -> LocalObjects& {
        switch (visibility) {
        case RemoteObjectVisibility::PRIVATE:
            return private_local_objects_;
        case RemoteObjectVisibility::PUBLIC:
            return public_local_objects_;
        }
        THROW_OR_ABORT("Unknown local object visibility");
    }();
    if (!objects.emplace(
        next_local_object_id_,
        object,
        CURRENT_SOURCE_LOCATION).second)
    {
        verbose_abort("Could not add private local object");
    }
    return {local_site_id_, next_local_object_id_++};
}

void IncrementalRemoteObjects::add_remote_object(
    const RemoteObjectId& id,
    const DanglingBaseClassRef<IIncrementalObject>& object,
    RemoteObjectVisibility visibility)
{
    if (id.site_id == local_site_id_) {
        THROW_OR_ABORT("Attempt to add a local object as remote");
    }
    auto& objects = [&]() -> RemoteObjects& {
        switch (visibility) {
        case RemoteObjectVisibility::PRIVATE:
            return private_remote_objects_;
        case RemoteObjectVisibility::PUBLIC:
            return public_remote_objects_;
        }
        THROW_OR_ABORT("Unknown remote object visibility");
    }();
    if (!objects.emplace(id, object, CURRENT_SOURCE_LOCATION).second) {
        THROW_OR_ABORT("Could not add remote object: " + id.to_displayname());
    }
}

DanglingBaseClassPtr<IIncrementalObject> IncrementalRemoteObjects::try_get(const RemoteObjectId& id) const {
    if (id.site_id == local_site_id_) {
        if (auto it = private_local_objects_.find(id.object_id); it != private_local_objects_.end()) {
            return it->second.object().ptr();
        }
        if (auto it = public_local_objects_.find(id.object_id); it != public_local_objects_.end()) {
            return it->second.object().ptr();
        }
        return nullptr;
    } else {
        if (auto it = private_remote_objects_.find(id); it != private_remote_objects_.end()) {
            return it->second.object().ptr();
        }
        if (auto it = public_remote_objects_.find(id); it != public_remote_objects_.end()) {
            return it->second.object().ptr();
        }
        return nullptr;
    }
}

bool IncrementalRemoteObjects::try_remove(const RemoteObjectId& id) {
    // If the local time is not set, this means that no transmission has taken
    // place yet, and the deleted objects need not be updated.
    if (local_time_.initialized()) {
        deleted_objects_.try_emplace(id, local_time());
    }
    auto o = try_get(id);
    if ((o == nullptr) || !global_object_pool.contains(o.get())) {
        return false;
    }
    global_object_pool.remove(o.release());
    return true;
}

const DeletedObjects& IncrementalRemoteObjects::deleted_objects() const {
    return deleted_objects_;
}

void IncrementalRemoteObjects::forget_old_deleted_objects() {
    deleted_objects_.forget_old_entries(local_time());
}

const LocalObjects& IncrementalRemoteObjects::private_local_objects() const {
    return private_local_objects_;
}

const LocalObjects& IncrementalRemoteObjects::public_local_objects() const {
    return public_local_objects_;
}

const RemoteObjects& IncrementalRemoteObjects::public_remote_objects() const {
    return public_remote_objects_;
}

void IncrementalRemoteObjects::print(std::ostream& ostr) const {
    ostr <<
        "#deleted: " << deleted_objects_.size() <<
        ", #private local: " << private_local_objects_.size() <<
        ", #public local: " << public_local_objects_.size() <<
        ", #private remote: " << private_remote_objects_.size() <<
        ", #public remote: " << public_remote_objects_.size();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const IncrementalRemoteObjects& objects) {
    objects.print(ostr);
    return ostr;
}
