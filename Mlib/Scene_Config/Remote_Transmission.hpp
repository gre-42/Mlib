#pragma once
#include <Mlib/Physics/Units.hpp>
#include <chrono>
#include <cstdint>
#include <ratio>

namespace Mlib {

using RemoteTimeRatio = std::milli;
static constexpr const float REMOTE_TIME_UNIT = (RemoteTimeRatio::num * seconds) / RemoteTimeRatio::den;
using RemoteTimeCount = uint16_t;
using DatagramIndexType = uint16_t;
using TransmissionHistoryType = uint8_t;
using TransmittedFieldsType = uint8_t;
using NSitesType = uint8_t;
using NUnknownType = uint16_t;
using NDeletedType = uint16_t;
using NShotsType = uint16_t;
using NSelectNextVehicleEventsType = uint8_t;
using StringLengthType = uint8_t;
using FragmentGroupType = uint16_t;
using FragmentIndexType = uint8_t;
// WebTransport allows at least 1'200 bytes.
// The following calculation assumes no magic byte is present in the fragment header.
static constexpr const size_t MAX_FRAGMENT_BYTES = 1'200 - sizeof(FragmentGroupType) - 2 * sizeof(FragmentIndexType);
static constexpr const auto FRAGMENT_TIMEOUT = std::chrono::milliseconds{500};

}
