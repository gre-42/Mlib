#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <cstdint>
#include <iosfwd>

namespace Mlib {

struct RemoteObjectId;
class TransmissionHistoryReader;
class TransmissionHistoryWriter;
enum class KnownFields: uint32_t;
enum class TransmittedFields: uint32_t;
enum class ProxyTasks;

class IIncrementalObject: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual ~IIncrementalObject() = default;
    virtual void read(
        std::istream& istr,
        const RemoteObjectId& remote_object_id,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) = 0;
    virtual void write(
        std::ostream& ostr,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        TransmissionHistoryWriter& transmission_history_writer) = 0;
};

}
