#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <cstdint>
#include <iosfwd>

namespace Mlib {

enum class KnownFields: uint32_t;
class TransmissionHistoryWriter;
enum class ProxyTasks;
enum class TransmittedFields: uint32_t;
struct RemoteObjectId;

class IIncrementalObject: public virtual DestructionNotifier, public virtual DanglingBaseClass {
public:
    virtual ~IIncrementalObject() = default;
    virtual void read(
        std::istream& istr,
        TransmittedFields transmitted_fields) = 0;
    virtual void write(
        std::ostream& ostr,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        TransmissionHistoryWriter& transmission_history_writer) = 0;
};

}
