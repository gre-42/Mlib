#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <cstdint>

namespace Mlib {

struct RemoteObjectId;
class BinaryBitwiseWordsReader;
class BinaryBitwiseWordsWriter;
class ProxyObjectsCaches;
class TransmissionHistoryReader;
class TransmissionHistoryWriter;
enum class KnownFields;
enum class TransmittedFields: TransmittedFieldsType;
enum class ProxyTasks;

class IIncrementalObject: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual ~IIncrementalObject() = default;
    virtual std::string name() const = 0;
    virtual int32_t priority() const = 0;
    virtual void read(
        BinaryBitwiseWordsReader& reader,
        RemoteSiteId sender_site_id,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        ProxyObjectsCaches& proxy_objects_caches,
        TransmissionHistoryReader& transmission_history_reader) = 0;
    virtual void write(
        BinaryBitwiseWordsWriter& writer,
        RemoteSiteId receiver_site_id,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        ProxyObjectsCaches& proxy_objects_caches,
        TransmissionHistoryWriter& transmission_history_writer) = 0;
};

}
