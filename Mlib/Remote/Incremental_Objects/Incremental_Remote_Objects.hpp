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

class IncrementalRemoteObjects: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    explicit IncrementalRemoteObjects(RemoteSiteId site_id);
    ~IncrementalRemoteObjects();
    RemoteSiteId site_id() const;
    void add_local_object(const DanglingBaseClassRef<IIncrementalObject>& object);
    void add_remote_object(const RemoteObjectId& id, const DanglingBaseClassRef<IIncrementalObject>& object);
    DanglingBaseClassPtr<IIncrementalObject> try_get(const RemoteObjectId& id) const;
    const LocalObjects& local_objects() const;
    const RemoteObjects& remote_objects() const;
    void print(std::ostream& ostr) const;

private:
    RemoteSiteId site_id_;
    LocalObjectId next_local_object_id_;
    LocalObjects local_objects_;
    RemoteObjects remote_objects_;
};

std::ostream& operator << (std::ostream& ostr, const IncrementalRemoteObjects& objects);

}
