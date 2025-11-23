#include "Transmission_History.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Scene_Config/Remote_Event_History_Duration.hpp>

using namespace Mlib;

TransmissionHistoryReader::TransmissionHistoryReader(
    std::chrono::steady_clock::time_point base_time)
    : base_time_{ base_time }
{}

TransmissionHistoryReader::~TransmissionHistoryReader() = default;

RemoteObjectId TransmissionHistoryReader::read_remote_object_id(
    std::istream& istr,
    TransmittedFields transmitted_fields,
    IoVerbosity verbosity)
{
    if (!any(transmitted_fields & TransmittedFields::SITE_ID)) {
        if (!site_id_.has_value()) {
            THROW_OR_ABORT("Site ID neither set in history nor in current transmission");
        }
        return RemoteObjectId{
            *site_id_,
            read_binary<LocalObjectId>(istr, "object ID", verbosity)
        };
    } else {
        auto remote_object_id = read_binary<RemoteObjectId>(istr, "remote object ID", verbosity);
        site_id_ = remote_object_id.site_id;
        return remote_object_id;
    }
}

std::chrono::steady_clock::time_point TransmissionHistoryReader::read_time(
    std::istream& istr,
    IoVerbosity verbosity) const
{
    if (base_time_ == std::chrono::steady_clock::time_point()) {
        THROW_OR_ABORT("Attempt to read time, but no base time was set");
    }
    auto offset = read_binary<RemoteEventHistoryOffset>(istr, "remote time offset", verbosity);
    if (offset >= base_time_.time_since_epoch()) {
        THROW_OR_ABORT("Time offset larger or equal the time since epoch");
    }
    return base_time_ - offset;
}

TransmissionHistoryWriter::TransmissionHistoryWriter(
    std::chrono::steady_clock::time_point base_time)
    : base_time_{ base_time }
    , history_{ TransmissionHistory::NONE }
{}

TransmissionHistoryWriter::~TransmissionHistoryWriter() = default;

void TransmissionHistoryWriter::write_remote_object_id(
    std::ostream& ostr,
    const RemoteObjectId& remote_object_id,
    TransmittedFields transmitted_fields)
{
    if (any(transmitted_fields & TransmittedFields::SITE_ID)) {
        THROW_OR_ABORT("Transmitted fields unexpectedly have the SITE_ID flag set");
    }
    if (any(history_ & TransmissionHistory::SITE_ID)) {
        write_binary(ostr, transmitted_fields, "transmitted fields");
        write_binary(ostr, remote_object_id.object_id, "object ID");
    } else {
        write_binary(ostr, transmitted_fields | TransmittedFields::SITE_ID, "transmitted fields");
        write_binary(ostr, remote_object_id, "remote object ID");
        history_ |= TransmissionHistory::SITE_ID;
    }
}

void TransmissionHistoryWriter::write_time(
    std::ostream& ostr,
    std::chrono::steady_clock::time_point time) const
{
    if (base_time_ == std::chrono::steady_clock::time_point()) {
        THROW_OR_ABORT("Attempt to write time, but no base time was set");
    }
    auto offset = base_time_ - time;
    if (offset.count() < 0) {
        THROW_OR_ABORT("Time is later than base time");
    }
    if (offset > REMOTE_EVENT_HISTORY_DURATION) {
        THROW_OR_ABORT("Time offset is larger than the maximum event history duration");
    }
    auto remote_offset = std::chrono::duration_cast<RemoteEventHistoryOffset>(offset);
    write_binary(ostr, remote_offset.count(), "remote time offset");
}
