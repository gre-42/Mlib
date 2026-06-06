#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>

namespace Mlib {

enum class TransmittedFields: TransmittedFieldsType;
enum class ProxyTasks;
struct RemoteObjectId;
class IIncrementalObject;
class BinaryBitwiseWordsReader;
class TransmissionHistoryReader;

class IIncrementalObjectFactory: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual DanglingBaseClassPtr<IIncrementalObject> try_create_shared_object(
        BinaryBitwiseWordsReader& reader,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) = 0;
};

}
