#include "Transmission_History.hpp"
#include <Mlib/Os/Io/Serialize/Serialize.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Scene_Config/Remote_Event_History_Duration.hpp>

using namespace Mlib;

TransmissionHistoryReader::TransmissionHistoryReader(
    const LocalSceneLevel& home_scene_level,
    std::chrono::steady_clock::time_point base_time)
    : home_scene_level{ home_scene_level }
    , base_time_{ base_time }
{}

TransmissionHistoryReader::~TransmissionHistoryReader() = default;

RemoteObjectId TransmissionHistoryReader::read_remote_object_id(
    BinaryBitwiseWordsReader& reader,
    TransmittedFields transmitted_fields)
{
    if (!any(transmitted_fields & TransmittedFields::SITE_ID)) {
        if (!site_id_.has_value()) {
            throw std::runtime_error("Site ID neither set in history nor in current transmission");
        }
        return RemoteObjectId{
            *site_id_,
            reader.read_binary<LocalObjectId>("object ID")
        };
    } else {
        auto remote_object_id = reader.deserialize<RemoteObjectId>("remote object ID");
        site_id_ = remote_object_id.site_id;
        return remote_object_id;
    }
}

std::chrono::steady_clock::time_point TransmissionHistoryReader::read_time(
    BinaryBitwiseWordsReader& reader) const
{
    if (base_time_ == std::chrono::steady_clock::time_point()) {
        throw std::runtime_error("Attempt to read time, but no base time was set");
    }
    auto offset = reader.read_binary<RemoteEventHistoryOffset>("remote time offset");
    if (offset >= base_time_.time_since_epoch()) {
        throw std::runtime_error("Time offset larger or equal the time since epoch");
    }
    return base_time_ - offset;
}

std::chrono::steady_clock::time_point TransmissionHistoryReader::base_time() const {
    return base_time_;
}

TransmissionHistoryWriter::TransmissionHistoryWriter(
    std::chrono::steady_clock::time_point base_time,
    uint32_t datagram_counter)
    : base_time_{ base_time }
    , datagram_counter_{ datagram_counter }
    , history_{ TransmissionHistory::NONE }
{}

TransmissionHistoryWriter::~TransmissionHistoryWriter() = default;

void TransmissionHistoryWriter::write_remote_object_id(
    BinaryBitwiseWordsWriter& writer,
    const RemoteObjectId& remote_object_id,
    TransmittedFields transmitted_fields)
{
    if (any(transmitted_fields & TransmittedFields::SITE_ID)) {
        throw std::runtime_error("Transmitted fields unexpectedly have the SITE_ID flag set");
    }
    auto site_id = [this](){
        if (!site_id_.has_value()) {
            throw std::runtime_error("Site ID not yet transmitted");
        }
        return *site_id_;
    };
    if (any(history_ & TransmissionHistory::SITE_ID) &&
        (site_id() == remote_object_id.site_id))
    {
        writer.write_binary(transmitted_fields, "transmitted fields");
        writer.write_binary(remote_object_id.object_id, "object ID");
    } else {
        site_id_.emplace(remote_object_id.site_id);
        writer.write_binary(transmitted_fields | TransmittedFields::SITE_ID, "transmitted fields");
        writer.serialize(remote_object_id, "remote object ID");
        history_ |= TransmissionHistory::SITE_ID;
    }
}

void TransmissionHistoryWriter::write_time(
    BinaryBitwiseWordsWriter& writer,
    std::chrono::steady_clock::time_point time) const
{
    if (base_time_ == std::chrono::steady_clock::time_point()) {
        throw std::runtime_error("Attempt to write time, but no base time was set");
    }
    auto offset = base_time_ - time;
    if (offset.count() < 0) {
        throw std::runtime_error("Time is later than base time");
    }
    if (offset > REMOTE_EVENT_HISTORY_DURATION) {
        throw std::runtime_error("Time offset is larger than the maximum event history duration");
    }
    auto remote_offset = std::chrono::duration_cast<RemoteEventHistoryOffset>(offset);
    writer.write_binary(remote_offset.count(), "remote time offset");
}

uint32_t TransmissionHistoryWriter::datagram_counter() const {
    return datagram_counter_;
}
