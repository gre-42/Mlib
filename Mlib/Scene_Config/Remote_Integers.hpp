#pragma once
#include <Mlib/Physics/Units.hpp>
#include <cstdint>
#include <ratio>

namespace Mlib {

using RemoteTimeRatio = std::milli;
static constexpr const float REMOTE_TIME_UNIT = (RemoteTimeRatio::num * seconds) / RemoteTimeRatio::den;
using RemoteTimeCount = uint16_t;
using DatagramIndexType = uint16_t;
using TransmissionHistoryType = uint8_t;
using TransmittedFieldsType = uint8_t;
using RemoteSiteId = uint8_t;
using LocalObjectId = uint16_t;
using RemoteSceneObjectUnderlyingType = uint8_t;
using NUnknownType = uint16_t;
using NDeletedType = uint16_t;
using NShotsType = uint16_t;
using NSelectNextVehicleEventsType = uint8_t;
using NUserCountType = uint8_t;
using UserStatusType = uint8_t;
using StringLengthType = uint8_t;
using ReloadCountType = uint8_t;
using SessionIdType = uint32_t;
using SkillsType = uint8_t;

}
