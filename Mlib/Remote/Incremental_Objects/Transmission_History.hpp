#pragma once
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <cstdint>
#include <iosfwd>
#include <optional>

namespace Mlib {

struct RemoteObjectId;
enum class IoVerbosity;
enum class TransmittedFields: uint32_t;

enum class TransmissionHistory: uint32_t {
    NONE = 0,
    SITE_ID = 1 << 1,
    END = 1 << 2
};

inline bool any(TransmissionHistory fields) {
    return fields != TransmissionHistory::NONE;
}

inline TransmissionHistory operator & (TransmissionHistory a, TransmissionHistory b) {
    return (TransmissionHistory)((int)a & (int)b);
}

inline TransmissionHistory operator | (TransmissionHistory a, TransmissionHistory b) {
    return (TransmissionHistory)((int)a | (int)b);
}

inline TransmissionHistory& operator |= (TransmissionHistory& a, TransmissionHistory b) {
    (int&)a |= (int)b;
    return a;
}

class TransmissionHistoryReader {
public:
    TransmissionHistoryReader();
    ~TransmissionHistoryReader();
    RemoteObjectId read(
        std::istream& istr,
        TransmittedFields transmitted_fields,
        IoVerbosity verbosity);
private:
    std::optional<RemoteSiteId> site_id_;
};

class TransmissionHistoryWriter {
public:
    TransmissionHistoryWriter();
    ~TransmissionHistoryWriter();
    void write(
        std::ostream& ostr,
        const RemoteObjectId& remote_object_id,
        TransmittedFields transmitted_fields);
private:
    TransmissionHistory history_;
};

}
