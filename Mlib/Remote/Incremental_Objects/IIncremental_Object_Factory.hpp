#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>

namespace Mlib {

enum class ObjectLifetimeStatus;
enum class ProxyTasks;
enum class TransmittedFields: TransmittedFieldsType;
struct RemoteObjectId;
class BinaryBitwiseWordsReader;
class IIncrementalObject;
class ProxyObjectsCaches;
class TransmissionHistoryReader;

class IIncrementalObjectFactory: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual DanglingBaseClassPtr<IIncrementalObject> try_create_shared_object(
        BinaryBitwiseWordsReader& reader,
        RemoteSiteId sender_site_id,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        ObjectLifetimeStatus lifetime_status,
        ProxyObjectsCaches& proxy_objects_caches,
        TransmissionHistoryReader& transmission_history_reader) = 0;
};

}
