#pragma once
#include <Mlib/Memory/Dangling_Unordered_Set.hpp>
#include <Mlib/Memory/Dangling_Value_Unordered_Map.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <iosfwd>
#include <set>

namespace Mlib {

class IIncrementalObject;

using DeletedObjects = std::set<RemoteObjectId>;
using LocalObjects = DanglingValueUnorderedMap<LocalObjectId, IIncrementalObject>;
using RemoteObjects = DanglingValueUnorderedMap<RemoteObjectId, IIncrementalObject>;

enum class RemoteObjectVisibility {
    PRIVATE,
    PUBLIC
};

class IncrementalRemoteObjects: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    explicit IncrementalRemoteObjects(RemoteSiteId local_site_id);
    ~IncrementalRemoteObjects();
    RemoteSiteId local_site_id() const;
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
    const LocalObjects& private_local_objects() const;
    const LocalObjects& public_local_objects() const;
    const RemoteObjects& public_remote_objects() const;
    void print(std::ostream& ostr) const;

private:
    RemoteSiteId local_site_id_;
    DeletedObjects deleted_objects_;
    LocalObjectId next_local_object_id_;
    LocalObjects private_local_objects_;
    LocalObjects public_local_objects_;
    RemoteObjects private_remote_objects_;
    RemoteObjects public_remote_objects_;
};

std::ostream& operator << (std::ostream& ostr, const IncrementalRemoteObjects& objects);

}
