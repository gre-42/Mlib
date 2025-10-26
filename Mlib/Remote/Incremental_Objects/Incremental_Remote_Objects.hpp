#pragma once
#include <Mlib/Memory/Dangling_Unordered_Set.hpp>
#include <Mlib/Memory/Dangling_Value_Unordered_Map.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <iosfwd>

namespace Mlib {

class IIncrementalObject;

using SharedObjects = DanglingValueUnorderedMap<RemoteObjectId, IIncrementalObject>;

class IncrementalRemoteObjects: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    IncrementalRemoteObjects();
    ~IncrementalRemoteObjects();
    void add_new_object(DanglingBaseClassRef<IIncrementalObject> object);
    void add_existing_object(RemoteObjectId id, DanglingBaseClassRef<IIncrementalObject> object);
    DanglingBaseClassPtr<IIncrementalObject> try_get(RemoteObjectId id) const;
    SharedObjects::iterator begin();
    SharedObjects::iterator end();
    size_t size() const;
    void print(std::ostream& ostr) const;

private:
    RemoteObjectId next_object_id_;
    SharedObjects objects_;
};

std::ostream& operator << (std::ostream& ostr, const IncrementalRemoteObjects& objects);

}
