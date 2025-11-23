#pragma once
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <chrono>
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
    explicit TransmissionHistoryReader(
        std::chrono::steady_clock::time_point base_time);
    ~TransmissionHistoryReader();
    RemoteObjectId read_remote_object_id(
        std::istream& istr,
        TransmittedFields transmitted_fields,
        IoVerbosity verbosity);
    std::chrono::steady_clock::time_point read_time(
        std::istream& istr,
        IoVerbosity verbosity) const;
private:
    std::chrono::steady_clock::time_point base_time_;
    std::optional<RemoteSiteId> site_id_;
};

class TransmissionHistoryWriter {
public:
    explicit TransmissionHistoryWriter(
        std::chrono::steady_clock::time_point base_time);
    ~TransmissionHistoryWriter();
    void write_remote_object_id(
        std::ostream& ostr,
        const RemoteObjectId& remote_object_id,
        TransmittedFields transmitted_fields);
    void write_time(
        std::ostream& ostr,
        std::chrono::steady_clock::time_point time) const;
private:
    std::chrono::steady_clock::time_point base_time_;
    TransmissionHistory history_;
};

}
