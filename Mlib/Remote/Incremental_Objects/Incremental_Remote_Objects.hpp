#pragma once
#include <Mlib/Memory/Dangling_Value_Map.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Remote/Events/Events_And_Times.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <Mlib/Time/Time_And_Pause.hpp>
#include <chrono>
#include <iosfwd>
#include <map>

namespace Mlib {

class IIncrementalObject;
class SceneLevelSelector;

using DeletedObjects = EventsAndTimes<RemoteObjectId, std::chrono::steady_clock::time_point>;
using LocalObjects = DanglingValueMap<LocalObjectId, IIncrementalObject>;
using RemoteObjects = DanglingValueMap<RemoteObjectId, IIncrementalObject>;

enum class RemoteObjectVisibility {
    PRIVATE,
    PUBLIC
};

class IncrementalRemoteObjects: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    explicit IncrementalRemoteObjects(
        RemoteSiteId local_site_id,
        const DanglingBaseClassRef<SceneLevelSelector>& local_scene_level_selector);
    ~IncrementalRemoteObjects();
    RemoteSiteId local_site_id() const;
    std::chrono::steady_clock::time_point local_time() const;
    PauseStatus pause_status() const;
    void set_local_time(const TimeAndPause<std::chrono::steady_clock::time_point>& time);
    DanglingBaseClassRef<SceneLevelSelector> local_scene_level_selector() const;
    RemoteObjectId add_local_object(
        const DanglingBaseClassRef<IIncrementalObject>& object,
        RemoteObjectVisibility visibility);
    void add_remote_object(
        const RemoteObjectId& id,
        const DanglingBaseClassRef<IIncrementalObject>& object,
        RemoteObjectVisibility visibility);
    DanglingBaseClassPtr<IIncrementalObject> try_get(const RemoteObjectId& id) const;
    bool try_remove(const RemoteObjectId& id);
    const DeletedObjects& deleted_objects() const;
    void forget_old_deleted_objects();
    const LocalObjects& private_local_objects() const;
    const LocalObjects& public_local_objects() const;
    const RemoteObjects& public_remote_objects() const;
    void print(std::ostream& ostr) const;

private:
    RemoteSiteId local_site_id_;
    TimeAndPause<std::chrono::steady_clock::time_point> local_time_;
    DanglingBaseClassRef<SceneLevelSelector> local_scene_level_selector_;
    DeletedObjects deleted_objects_;
    LocalObjectId next_local_object_id_;
    LocalObjects private_local_objects_;
    LocalObjects public_local_objects_;
    RemoteObjects private_remote_objects_;
    RemoteObjects public_remote_objects_;
};

std::ostream& operator << (std::ostream& ostr, const IncrementalRemoteObjects& objects);

}
