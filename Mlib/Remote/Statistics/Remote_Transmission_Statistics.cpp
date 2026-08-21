#include "Remote_Transmission_Statistics.hpp"
#include <Mlib/Math/Is_Newer.hpp>
#include <stdexcept>

using namespace Mlib;

RemoteTransmissionStatistics::RemoteTransmissionStatistics()
    : old_version_{0}
    , ndropped_{0}
    , nout_of_order_{0}
{}

RemoteTransmissionStatistics::~RemoteTransmissionStatistics() = default;

void RemoteTransmissionStatistics::notify_datagram(DatagramIndexType version) {
    // Note: This implementation does not correctly handle duplicate datagrams.
    auto distance = minus_modulo(version, old_version_);
    if (distance > 1) {
        ndropped_ += DatagramIndexType(distance - 1);
    }
    if ((distance < 0) && (ndropped_ > 0)) {
        ndropped_ -= 1;
        ++nout_of_order_;
    }
    old_version_ = version;
}

void RemoteTransmissionStatistics::print(std::ostream& ostr) const {
    ostr <<
        "#dropped: " << ndropped_ <<
        ", #out of order: " << nout_of_order_;

}

std::ostream& Mlib::operator << (
    std::ostream& ostr,
    const RemoteTransmissionStatistics& stats)
{
    stats.print(ostr);
    return ostr;
}
