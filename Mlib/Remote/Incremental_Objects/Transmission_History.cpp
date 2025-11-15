#include "Transmission_History.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>

using namespace Mlib;

TransmissionHistoryReader::TransmissionHistoryReader() = default;

TransmissionHistoryReader::~TransmissionHistoryReader() = default;

RemoteObjectId TransmissionHistoryReader::read(
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

TransmissionHistoryWriter::TransmissionHistoryWriter()
    : history_{ TransmissionHistory::NONE }
{}

TransmissionHistoryWriter::~TransmissionHistoryWriter() = default;

void TransmissionHistoryWriter::write(
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
