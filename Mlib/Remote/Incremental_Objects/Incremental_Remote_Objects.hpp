#pragma once
#include <Mlib/Memory/Dangling_Unordered_Set.hpp>
#include <Mlib/Memory/Dangling_Value_Unordered_Map.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <iosfwd>

namespace Mlib {

class IIncrementalObject;

using LocalObjects = DanglingValueUnorderedMap<LocalObjectId, IIncrementalObject>;
using RemoteObjects = DanglingValueUnorderedMap<RemoteObjectId, IIncrementalObject>;

enum class RemoteObjectVisibility {
    PRIVATE,
    PUBLIC
};

class IncrementalRemoteObjects: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    explicit IncrementalRemoteObjects(RemoteSiteId site_id);
    ~IncrementalRemoteObjects();
    RemoteSiteId site_id() const;
    void add_local_object(
        const DanglingBaseClassRef<IIncrementalObject>& object,
        RemoteObjectVisibility visibility);
    void add_remote_object(
        const RemoteObjectId& id,
        const DanglingBaseClassRef<IIncrementalObject>& object,
        RemoteObjectVisibility visibility);
    DanglingBaseClassPtr<IIncrementalObject> try_get(const RemoteObjectId& id) const;
    const LocalObjects& private_local_objects() const;
    const LocalObjects& public_local_objects() const;
    const RemoteObjects& public_remote_objects() const;
    void print(std::ostream& ostr) const;

private:
    RemoteSiteId site_id_;
    LocalObjectId next_local_object_id_;
    LocalObjects private_local_objects_;
    LocalObjects public_local_objects_;
    RemoteObjects private_remote_objects_;
    RemoteObjects public_remote_objects_;
};

std::ostream& operator << (std::ostream& ostr, const IncrementalRemoteObjects& objects);

}
