#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <chrono>
#include <cstdint>
#include <optional>

namespace Mlib {

struct LocalSceneLevel;
struct RemoteObjectId;
enum class TransmittedFields: TransmittedFieldsType;
class BinaryBitwiseWordsReader;
class BinaryBitwiseWordsWriter;

enum class TransmissionHistory: TransmissionHistoryType {
    NONE = 0,
    SITE_ID = 1 << 1,
    END = 1 << 2
};

inline bool any(TransmissionHistory fields) {
    return fields != TransmissionHistory::NONE;
}

inline TransmissionHistory operator & (TransmissionHistory a, TransmissionHistory b) {
    return (TransmissionHistory)((TransmissionHistoryType)a & (TransmissionHistoryType)b);
}

inline TransmissionHistory operator | (TransmissionHistory a, TransmissionHistory b) {
    return (TransmissionHistory)((TransmissionHistoryType)a | (TransmissionHistoryType)b);
}

inline TransmissionHistory& operator |= (TransmissionHistory& a, TransmissionHistory b) {
    (TransmissionHistoryType&)a |= (TransmissionHistoryType)b;
    return a;
}

class TransmissionHistoryReader {
public:
    explicit TransmissionHistoryReader(
        const LocalSceneLevel& home_scene_level,
        std::chrono::steady_clock::time_point base_time);
    ~TransmissionHistoryReader();
    RemoteObjectId read_remote_object_id(
        BinaryBitwiseWordsReader& reader,
        TransmittedFields transmitted_fields);
    std::chrono::steady_clock::time_point read_time(
        BinaryBitwiseWordsReader& reader) const;
    std::chrono::steady_clock::time_point base_time() const;
    const LocalSceneLevel& home_scene_level;
private:
    std::chrono::steady_clock::time_point base_time_;
    std::optional<RemoteSiteId> site_id_;
};

class TransmissionHistoryWriter {
public:
    TransmissionHistoryWriter(
        std::chrono::steady_clock::time_point base_time,
        uint32_t datagram_counter);
    ~TransmissionHistoryWriter();
    void write_remote_object_id(
        BinaryBitwiseWordsWriter& writer,
        const RemoteObjectId& remote_object_id,
        TransmittedFields transmitted_fields);
    void write_time(
        BinaryBitwiseWordsWriter& writer,
        std::chrono::steady_clock::time_point time) const;
    uint32_t datagram_counter() const;
private:
    std::chrono::steady_clock::time_point base_time_;
    uint32_t datagram_counter_;
    std::optional<RemoteSiteId> site_id_;
    TransmissionHistory history_;
};

}
